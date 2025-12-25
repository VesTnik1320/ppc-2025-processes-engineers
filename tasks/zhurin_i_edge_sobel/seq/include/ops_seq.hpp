#pragma once

#include <vector>

#include "task/include/task.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

class ZhurinIEdgeSobelSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit ZhurinIEdgeSobelSEQ(const InType &in);

 private:
  bool ValidationImpl() override {
    return true;
  }
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override {
    return true;
  }

  [[nodiscard]] int GradientX(int x, int y) const;
  [[nodiscard]] int GradientY(int x, int y) const;

  int height_ = 0;
  int width_ = 0;
  int threshold_ = 0;
  std::vector<int> input_pixels_;
};

}  // namespace zhurin_i_edge_sobel
