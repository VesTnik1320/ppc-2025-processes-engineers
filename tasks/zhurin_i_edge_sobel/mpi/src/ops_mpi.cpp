#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_sobel_edge {

static constexpr std::array<std::array<int, 3>, 3> H_KERNEL = {
    std::array<int, 3>{-1, 0, 1}, std::array<int, 3>{-2, 0, 2}, std::array<int, 3>{-1, 0, 1}};

static constexpr std::array<std::array<int, 3>, 3> V_KERNEL = {
    std::array<int, 3>{-1, -2, -1}, std::array<int, 3>{0, 0, 0}, std::array<int, 3>{1, 2, 1}};

MPIEdgeProcessor::MPIEdgeProcessor(const ImageTuple &input) {
  SetTypeOfTask(getTypeMarker());

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    GetInput() = input;
    sourceImage = std::get<0>(GetInput());
    rows = std::get<1>(GetInput());
    cols = std::get<2>(GetInput());
    cutoff = std::get<3>(GetInput());
  }

  GetOutput().clear();
}

bool MPIEdgeProcessor::ValidationImpl() {
  return rows > 0 && cols > 0 && cutoff >= 0;
}

bool MPIEdgeProcessor::PreProcessingImpl() {
  return true;
}

bool MPIEdgeProcessor::RunImpl() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  broadcastMetadata();
  splitImage();

  std::vector<int> local = processChunk();
  assembleResults(local);

  return true;
}

bool MPIEdgeProcessor::PostProcessingImpl() {
  return true;
}

void MPIEdgeProcessor::broadcastMetadata() {
  MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&cutoff, 1, MPI_INT, 0, MPI_COMM_WORLD);
}

// Гало-строки - БЕЗ ИЗМЕНЕНИЙ
void MPIEdgeProcessor::RowDistributionComputing(int world_rank, int world_size, int &base_rows, int &remainder,
                                                int &real_rows, int &need_top_halo, int &need_bottom_halo,
                                                int &total_rows) {
  base_rows = rows / world_size;
  remainder = rows % world_size;

  real_rows = base_rows + (world_rank < remainder ? 1 : 0);
  chunkRows = real_rows;

  need_top_halo = (world_rank > 0) ? 1 : 0;
  need_bottom_halo = (world_rank < (world_size - 1)) ? 1 : 0;

  total_rows = real_rows + need_top_halo + need_bottom_halo;
  chunkRowsWithBorder = total_rows;
  chunk.resize(static_cast<size_t>(total_rows) * cols, 0);
}

void MPIEdgeProcessor::SendParameters(int world_rank, int world_size, int base_rows, int remainder,
                                      std::vector<int> &real_rows_per_proc, std::vector<int> &send_counts,
                                      std::vector<int> &send_displs) const {
  if (world_rank == 0) {
    int current_row = 0;
    for (int dest = 0; dest < world_size; ++dest) {
      int dest_real_rows = base_rows + (dest < remainder ? 1 : 0);
      real_rows_per_proc[dest] = dest_real_rows;

      int dest_need_top_halo = (dest > 0) ? 1 : 0;
      int dest_need_bottom_halo = (dest < (world_size - 1)) ? 1 : 0;
      int start_row_with_halo = current_row - dest_need_top_halo;

      int end_row_with_halo = current_row + dest_real_rows + dest_need_bottom_halo - 1;
      end_row_with_halo = std::min(end_row_with_halo, rows - 1);

      int actual_rows = end_row_with_halo - start_row_with_halo + 1;

      send_counts[dest] = actual_rows * cols;
      send_displs[dest] = start_row_with_halo * cols;

      current_row += dest_real_rows;
    }
  }
}

void MPIEdgeProcessor::DataDistribution(int world_rank, const std::vector<int> &send_counts,
                                        const std::vector<int> &send_displs) {
  MPI_Scatterv(world_rank == 0 ? sourceImage.data() : nullptr, send_counts.data(), send_displs.data(), MPI_INT,
               chunk.data(), static_cast<int>(chunk.size()), MPI_INT, 0, MPI_COMM_WORLD);
}

void MPIEdgeProcessor::splitImage() {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int base, rem, real, topHalo, botHalo, total;

  RowDistributionComputing(rank, size, base, rem, real, topHalo, botHalo, total);

  std::vector<int> counts(size, 0), offsets(size, 0), rowsPerProc(size, 0);

  SendParameters(rank, size, base, rem, rowsPerProc, counts, offsets);
  DataDistribution(rank, counts, offsets);
}

std::vector<int> MPIEdgeProcessor::processChunk() {
  std::vector<int> result;

  if (chunkRows > 0) {
    result.resize(static_cast<size_t>(chunkRows) * cols, 0);

    for (int r = 0; r < chunkRows; ++r) {
      int globalR = r + (chunkRowsWithBorder > chunkRows ? 1 : 0);

      for (int c = 0; c < cols; ++c) {
        int dx = calcGradH(c, globalR);
        int dy = calcGradV(c, globalR);

        int val = static_cast<int>(std::sqrt(dx * dx + dy * dy));
        result[r * cols + c] = val > cutoff ? val : 0;
      }
    }
  }

  return result;
}

int MPIEdgeProcessor::calcGradH(int px, int py) {
  int accum = 0;

  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      int nx = px + dc;
      int ny = py + dr;

      if (nx >= 0 && nx < cols && ny >= 0 && ny < chunkRowsWithBorder) {
        accum += chunk[ny * cols + nx] * H_KERNEL[dr + 1][dc + 1];
      }
    }
  }

  return accum;
}

int MPIEdgeProcessor::calcGradV(int px, int py) {
  int accum = 0;

  for (int dr = -1; dr <= 1; ++dr) {
    for (int dc = -1; dc <= 1; ++dc) {
      int nx = px + dc;
      int ny = py + dr;

      if (nx >= 0 && nx < cols && ny >= 0 && ny < chunkRowsWithBorder) {
        accum += chunk[ny * cols + nx] * V_KERNEL[dr + 1][dc + 1];
      }
    }
  }

  return accum;
}

void MPIEdgeProcessor::assembleResults(const std::vector<int> &partial) {
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  std::vector<int> sizes(size);
  int localSize = static_cast<int>(partial.size());
  MPI_Allgather(&localSize, 1, MPI_INT, sizes.data(), 1, MPI_INT, MPI_COMM_WORLD);

  std::vector<int> starts(size);
  int total = 0;
  for (int i = 0; i < size; ++i) {
    starts[i] = total;
    total += sizes[i];
  }

  GetOutput().resize(total);

  MPI_Allgatherv(partial.empty() ? nullptr : partial.data(), localSize, MPI_INT, GetOutput().data(), sizes.data(),
                 starts.data(), MPI_INT, MPI_COMM_WORLD);
}

}  // namespace zhurin_i_sobel_edge
