#ifndef ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_
#define ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_

#include <cstring>  // Для memset
#include <tuple>
#include <vector>

namespace zhurin_i_ring_topology {

struct RingMessage {
  int source{0};
  int dest{0};
  std::vector<int> data{};
  bool go_clockwise{true};

  // Конструктор, который обнуляет всю структуру
  RingMessage() {
    source = 0;
    dest = 0;
    go_clockwise = true;
  }

  RingMessage(int src, int dst, std::vector<int> d, bool clockwise)
      : source(src), dest(dst), data(std::move(d)), go_clockwise(clockwise) {}

  // Явно обнуляем все байты при копировании
  RingMessage(const RingMessage &other)
      : source(other.source), dest(other.dest), data(other.data), go_clockwise(other.go_clockwise) {}

  RingMessage &operator=(const RingMessage &other) {
    source = other.source;
    dest = other.dest;
    data = other.data;
    go_clockwise = other.go_clockwise;
    return *this;
  }
};

using InType = RingMessage;
using OutType = std::vector<int>;
using TestType = std::tuple<int, RingMessage>;

}  // namespace zhurin_i_ring_topology

#endif
