#ifndef ZHURIN_I_EDGE_DETECTION_SEQ_HPP
#define ZHURIN_I_EDGE_DETECTION_SEQ_HPP

#include <vector>

#include "task/include/task.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_sobel_edge {

class SequentialEdgeDetector : public TaskInterface {
 public:
  static constexpr auto getTypeMarker() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit SequentialEdgeDetector(const ImageTuple &input);

 private:
  bool ValidationImpl() final;
  bool PreProcessingImpl() final;
  bool RunImpl() final;
  bool PostProcessingImpl() final;

  int computeHorizontal(int x, int y);
  int computeVertical(int x, int y);

  int height, width, limit;
  std::vector<int> pixels;
};

}  // namespace zhurin_i_sobel_edge

#endif
