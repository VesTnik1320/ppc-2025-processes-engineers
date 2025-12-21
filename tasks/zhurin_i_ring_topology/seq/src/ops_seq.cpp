#include "zhurin_i_ring_topology/seq/include/ops_seq.hpp"

#include <chrono>
#include <thread>
#include <vector>

#include "zhurin_i_ring_topology/common/include/common.hpp"

namespace zhurin_i_ring_topology {

ZhurinIRingTopologySEQ::ZhurinIRingTopologySEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {};
}

bool ZhurinIRingTopologySEQ::ValidationImpl() {
  const auto &input = GetInput();
  return input.source >= 0 && input.dest >= 0;
}

bool ZhurinIRingTopologySEQ::PreProcessingImpl() {
  GetOutput() = {};
  return true;
}

bool ZhurinIRingTopologySEQ::RunImpl() {
  const auto &input = GetInput();
  GetOutput() = input.data;
  if (input.source != input.dest) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  return true;
}

bool ZhurinIRingTopologySEQ::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_ring_topology
