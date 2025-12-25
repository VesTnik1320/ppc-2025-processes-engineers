#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace zhurin_i_edge_sobel {

ZhurinIEdgeSobelMPI::ZhurinIEdgeSobelMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    GetInput() = in;
    input_pixels_ = std::get<0>(in);
    height_ = std::get<1>(in);
    width_ = std::get<2>(in);
    threshold_ = std::get<3>(in);
  }

  GetOutput().clear();
}

bool ZhurinIEdgeSobelMPI::ValidationImpl() {
  return true;
}
bool ZhurinIEdgeSobelMPI::PreProcessingImpl() {
  return true;
}
bool ZhurinIEdgeSobelMPI::PostProcessingImpl() {
  return true;
}

bool ZhurinIEdgeSobelMPI::RunImpl() {
  BroadcastParameters();
  DistributeRows();
  auto local_result = LocalGradientsComputing();
  GatherResults(local_result);
  return true;
}

void ZhurinIEdgeSobelMPI::BroadcastParameters() {
  int params[3] = {height_, width_, threshold_};
  MPI_Bcast(params, 3, MPI_INT, 0, MPI_COMM_WORLD);
  height_ = params[0];
  width_ = params[1];
  threshold_ = params[2];
}

void ZhurinIEdgeSobelMPI::RowDistributionComputing(int world_rank, int world_size, int &base_rows, int &remainder,
                                                   int &real_rows, int &need_top_halo, int &need_bottom_halo,
                                                   int &total_rows) {
  base_rows = height_ / world_size;
  remainder = height_ % world_size;

  real_rows = base_rows + (world_rank < remainder ? 1 : 0);
  local_height_ = real_rows;

  need_top_halo = (world_rank > 0) ? 1 : 0;
  need_bottom_halo = (world_rank < world_size - 1) ? 1 : 0;

  total_rows = real_rows + need_top_halo + need_bottom_halo;
  local_height_with_halo_ = total_rows;

  local_pixels_.assign(static_cast<size_t>(total_rows) * width_, 0);
}

void ZhurinIEdgeSobelMPI::SendParameters(int world_rank, int world_size, int base_rows, int remainder,
                                         std::vector<int> &real_rows_per_proc, std::vector<int> &send_counts,
                                         std::vector<int> &send_displs) const {
  if (world_rank != 0) {
    return;
  }

  int current_row = 0;
  for (int proc_idx = 0; proc_idx < world_size; ++proc_idx) {
    int rows = base_rows + (proc_idx < remainder ? 1 : 0);
    real_rows_per_proc[proc_idx] = rows;

    int top = (proc_idx > 0) ? 1 : 0;
    int bottom = (proc_idx < world_size - 1) ? 1 : 0;

    int start = current_row - top;
    int end = std::min(current_row + rows + bottom - 1, height_ - 1);

    int count_rows = end - start + 1;
    send_counts[proc_idx] = count_rows * width_;
    send_displs[proc_idx] = start * width_;

    current_row += rows;
  }
}

void ZhurinIEdgeSobelMPI::DistributeRows() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int base = 0, rem = 0, real = 0, top = 0, bottom = 0, total = 0;
  RowDistributionComputing(rank, size, base, rem, real, top, bottom, total);

  std::vector<int> send_counts(static_cast<size_t>(size), 0);
  std::vector<int> send_displs(static_cast<size_t>(size), 0);
  std::vector<int> real_rows_per_proc(static_cast<size_t>(size), 0);

  SendParameters(rank, size, base, rem, real_rows_per_proc, send_counts, send_displs);

  MPI_Scatterv(rank == 0 ? input_pixels_.data() : nullptr, send_counts.data(), send_displs.data(), MPI_INT,
               local_pixels_.data(), total * width_, MPI_INT, 0, MPI_COMM_WORLD);
}

std::vector<int> ZhurinIEdgeSobelMPI::LocalGradientsComputing() {
  std::vector<int> result(static_cast<size_t>(local_height_) * width_, 0);
  int offset = (local_height_with_halo_ > local_height_) ? 1 : 0;

  for (int iy = 0; iy < local_height_; ++iy) {
    for (int ix = 0; ix < width_; ++ix) {
      int gx = GradientX(ix, iy + offset);
      int gy = GradientY(ix, iy + offset);
      int mag = static_cast<int>(std::sqrt((gx * gx) + (gy * gy)));  // скобки для приоритета
      result[iy * width_ + ix] = (mag > threshold_) ? mag : 0;
    }
  }
  return result;
}

int ZhurinIEdgeSobelMPI::GradientX(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < local_height_with_halo_) {
        sum += local_pixels_[(ny * width_) + nx] * kSobelX[ky + 1][kx + 1];  // скобки
      }
    }
  }
  return sum;
}

int ZhurinIEdgeSobelMPI::GradientY(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < local_height_with_halo_) {
        sum += local_pixels_[(ny * width_) + nx] * kSobelY[ky + 1][kx + 1];  // скобки
      }
    }
  }
  return sum;
}

void ZhurinIEdgeSobelMPI::GatherResults(const std::vector<int> &local_result) {
  int rank = 0, size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int local_size = static_cast<int>(local_result.size());
  std::vector<int> recv_counts(size);
  MPI_Allgather(&local_size, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

  std::vector<int> displs(size);
  int total_size = 0;
  for (int i = 0; i < size; ++i) {
    displs[i] = total_size;
    total_size += recv_counts[i];
  }

  GetOutput().resize(static_cast<size_t>(total_size));
  MPI_Allgatherv(local_result.data(), local_size, MPI_INT, GetOutput().data(), recv_counts.data(), displs.data(),
                 MPI_INT, MPI_COMM_WORLD);
}

void ZhurinIEdgeSobelMPI::LocalRowsComputing(int /*unused*/, int /*unused*/) {}
void ZhurinIEdgeSobelMPI::DataDistribution(int /*unused*/, const std::vector<int> & /*unused*/,
                                           const std::vector<int> & /*unused*/) {}

}  // namespace zhurin_i_edge_sobel
