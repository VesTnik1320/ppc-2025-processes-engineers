#include "zhurin_i_ring_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "zhurin_i_ring_topology/common/include/common.hpp"

namespace zhurin_i_ring_topology {

namespace {

bool InRing(int rank, int source, int dest, bool go_clockwise, int world_size) {
  if (source == dest) {
    return false;
  }

  if (rank < 0 || rank >= world_size || source < 0 || source >= world_size || dest < 0 || dest >= world_size) {
    return false;
  }

  if (go_clockwise) {
    if (source < dest) {
      return rank > source && rank <= dest;
    } else {
      return rank > source || rank <= dest;
    }
  } else {
    if (source > dest) {
      return rank < source && rank >= dest;
    } else {
      return rank < source || rank >= dest;
    }
  }
}

void SendAllInfo(int dest_rank, uint64_t data_size, const std::vector<int> &data, int size_tag = 0, int data_tag = 1) {
  if (dest_rank == MPI_PROC_NULL) {
    return;
  }

  MPI_Send(&data_size, 1, MPI_UINT64_T, dest_rank, size_tag, MPI_COMM_WORLD);
  if (data_size > 0U) {
    MPI_Send(data.data(), static_cast<int>(data_size), MPI_INT, dest_rank, data_tag, MPI_COMM_WORLD);
  }
}

void ReceiveAllInfo(int src_rank, uint64_t &data_size, std::vector<int> &data, int size_tag = 0, int data_tag = 1) {
  if (src_rank == MPI_PROC_NULL) {
    return;
  }

  MPI_Recv(&data_size, 1, MPI_UINT64_T, src_rank, size_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  if (data_size > 0U) {
    data.resize(static_cast<std::size_t>(data_size));
    MPI_Recv(data.data(), static_cast<int>(data_size), MPI_INT, src_rank, data_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void BroadcastResult(int rank, int root, std::vector<int> &output) {
  uint64_t data_size = 0;

  if (rank == root) {
    data_size = static_cast<uint64_t>(output.size());
  }

  MPI_Bcast(&data_size, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);

  if (rank != root) {
    output.resize(static_cast<std::size_t>(data_size));
  }

  if (data_size > 0U) {
    MPI_Bcast(output.data(), static_cast<int>(data_size), MPI_INT, root, MPI_COMM_WORLD);
  }
}

void SameSD(int rank, int source, const std::vector<int> &input_data, std::vector<int> &output) {
  uint64_t data_size = static_cast<uint64_t>(input_data.size());

  MPI_Bcast(&data_size, 1, MPI_UINT64_T, source, MPI_COMM_WORLD);

  if (rank == source) {
    output = input_data;
  } else {
    output.resize(static_cast<std::size_t>(data_size));
  }

  if (data_size > 0U) {
    MPI_Bcast(output.data(), static_cast<int>(data_size), MPI_INT, source, MPI_COMM_WORLD);
  }
}

void DataRoute(int rank, int source, int dest, bool go_clockwise, int world_size, const std::vector<int> &input_data,
               std::vector<int> &output) {
  std::vector<int> buffer;
  uint64_t data_size = 0;

  if (world_size == 1) {
    if (rank == source) {
      output = input_data;
    }
    return;
  }

  if (world_size == 2) {
    if (rank == source) {
      buffer = input_data;
      data_size = static_cast<uint64_t>(buffer.size());
      SendAllInfo(dest, data_size, buffer);
    }
    if (rank == dest) {
      ReceiveAllInfo(source, data_size, buffer);
      output = buffer;
    }
    return;
  }

  int left_neighbor = (rank - 1 + world_size) % world_size;
  int right_neighbor = (rank + 1) % world_size;

  int next_hop = go_clockwise ? right_neighbor : left_neighbor;
  int prev_hop = go_clockwise ? left_neighbor : right_neighbor;

  if (rank == source) {
    buffer = input_data;
    data_size = static_cast<uint64_t>(buffer.size());
    SendAllInfo(next_hop, data_size, buffer);
  }

  if (InRing(rank, source, dest, go_clockwise, world_size)) {
    ReceiveAllInfo(prev_hop, data_size, buffer);

    if (rank == dest) {
      output = buffer;
    } else {
      SendAllInfo(next_hop, data_size, buffer);
    }
  }
}

}  // namespace

ZhurinIRingTopologyMPI::ZhurinIRingTopologyMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().clear();
}

bool ZhurinIRingTopologyMPI::ValidationImpl() {
  const auto &input = GetInput();
  return input.source >= 0 && input.dest >= 0;
}

bool ZhurinIRingTopologyMPI::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool ZhurinIRingTopologyMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  const auto &input = GetInput();

  if (world_size == 1) {
    int effective_source = 0;

    SameSD(rank, effective_source, input.data, GetOutput());
    return true;
  }

  int effective_source = input.source % world_size;
  int effective_dest = input.dest % world_size;

  if (effective_source == effective_dest) {
    SameSD(rank, effective_source, input.data, GetOutput());
    return true;
  }

  int clockwise_distance = (effective_dest - effective_source + world_size) % world_size;
  int counter_distance = (effective_source - effective_dest + world_size) % world_size;
  bool go_clockwise = clockwise_distance <= counter_distance;

  DataRoute(rank, effective_source, effective_dest, go_clockwise, world_size, input.data, GetOutput());

  BroadcastResult(rank, effective_dest, GetOutput());
  return true;
}

bool ZhurinIRingTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_ring_topology
