#ifndef ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_
#define ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_

#include <tuple>
#include <vector>

namespace zhurin_i_ring_topology {

struct RingMessage {
  int source{};
  int dest{};
  std::vector<int> data;
  bool go_clockwise = true;

  RingMessage() = default;

  RingMessage(int s, int d, std::vector<int> dta, bool gc)
      : source(s), dest(d), data(std::move(dta)), go_clockwise(gc) {}
};

using InType = RingMessage;
using OutType = std::vector<int>;
using TestType = std::tuple<int, RingMessage>;

}  // namespace zhurin_i_ring_topology

#endif  // ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_
