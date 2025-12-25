#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <cmath>
#include <vector>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

const std::vector<std::vector<int>> kSobelX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};

const std::vector<std::vector<int>> kSobelY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

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

  GetOutput() = std::vector<int>();
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

  local_pixels_.assign(total_rows * width_, 0);
}

void ZhurinIEdgeSobelMPI::SendParameters(int world_rank, int world_size, int base_rows, int remainder,
                                         std::vector<int> &real_rows_per_proc, std::vector<int> &send_counts,
                                         std::vector<int> &send_displs) const {
  if (world_rank != 0) {
    return;
  }

  int current_row = 0;
  for (int p = 0; p < world_size; ++p) {
    int rows = base_rows + (p < remainder ? 1 : 0);
    real_rows_per_proc[p] = rows;

    int top = (p > 0) ? 1 : 0;
    int bottom = (p < world_size - 1) ? 1 : 0;

    int start = current_row - top;
    int end = std::min(current_row + rows + bottom - 1, height_ - 1);

    int count_rows = end - start + 1;
    send_counts[p] = count_rows * width_;
    send_displs[p] = start * width_;

    current_row += rows;
  }
}

void ZhurinIEdgeSobelMPI::DistributeRows() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int base, rem, real, top, bottom, total;
  RowDistributionComputing(rank, size, base, rem, real, top, bottom, total);

  std::vector<int> send_counts(size);
  std::vector<int> send_displs(size);
  std::vector<int> real_rows(size);

  SendParameters(rank, size, base, rem, real_rows, send_counts, send_displs);

  MPI_Scatterv(rank == 0 ? input_pixels_.data() : nullptr, send_counts.data(), send_displs.data(), MPI_INT,
               local_pixels_.data(), total * width_, MPI_INT, 0, MPI_COMM_WORLD);
}

std::vector<int> ZhurinIEdgeSobelMPI::LocalGradientsComputing() {
  std::vector<int> result(local_height_ * width_, 0);

  int offset = (local_height_with_halo_ > local_height_) ? 1 : 0;

  for (int y = 0; y < local_height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      int gx = GradientX(x, y + offset);
      int gy = GradientY(x, y + offset);
      int mag = static_cast<int>(std::sqrt(gx * gx + gy * gy));
      result[y * width_ + x] = (mag > threshold_) ? mag : 0;
    }
  }
  return result;
}

int ZhurinIEdgeSobelMPI::GradientX(int x, int y) {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < local_height_with_halo_) {
        sum += local_pixels_[ny * width_ + nx] * kSobelX[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

int ZhurinIEdgeSobelMPI::GradientY(int x, int y) {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < local_height_with_halo_) {
        sum += local_pixels_[ny * width_ + nx] * kSobelY[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

void ZhurinIEdgeSobelMPI::GatherResults(const std::vector<int> &local_result) {
  int rank = 0;
  int size = 0;
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

void ZhurinIEdgeSobelMPI::LocalRowsComputing(int, int) {}
void ZhurinIEdgeSobelMPI::DataDistribution(int, const std::vector<int> &, const std::vector<int> &) {}

}  // namespace zhurin_i_edge_sobel
