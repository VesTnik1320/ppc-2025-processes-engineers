#ifndef ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_
#define ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_

#include <cstring>
#include <tuple>
#include <utility>
#include <vector>

namespace zhurin_i_ring_topology {

struct RingMessage {
  int source{0};
  int dest{0};
  std::vector<int> data;
  bool go_clockwise{true};

  RingMessage() = default;

  RingMessage(int src, int dst, std::vector<int> d, bool clockwise)
      : source(src), dest(dst), data(std::move(d)), go_clockwise(clockwise) {}

  RingMessage(const RingMessage &other) = default;
  RingMessage &operator=(const RingMessage &other) = default;

  ~RingMessage() = default;
};

using InType = RingMessage;
using OutType = std::vector<int>;
using TestType = std::tuple<int, RingMessage>;

}  // namespace zhurin_i_ring_topology

#endif  // ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_
