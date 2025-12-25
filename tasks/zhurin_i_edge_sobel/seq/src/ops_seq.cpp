#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

const std::vector<std::vector<int>> kSobelX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const std::vector<std::vector<int>> kSobelY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

ZhurinIEdgeSobelSEQ::ZhurinIEdgeSobelSEQ(const InType &in)
    : height_(std::get<1>(in)), width_(std::get<2>(in)), threshold_(std::get<3>(in)), input_pixels_(std::get<0>(in)) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool ZhurinIEdgeSobelSEQ::PreProcessingImpl() {
  GetOutput().assign(static_cast<size_t>(height_) * width_, 0);
  return true;
}

bool ZhurinIEdgeSobelSEQ::RunImpl() {
  auto &output = GetOutput();

  for (int iy = 0; iy < height_; ++iy) {
    for (int ix = 0; ix < width_; ++ix) {
      int gx = GradientX(ix, iy);
      int gy = GradientY(ix, iy);
      int mag = static_cast<int>(std::sqrt((gx * gx) + (gy * gy)));
      output[(iy * width_) + ix] = (mag > threshold_) ? mag : 0;
    }
  }
  return true;
}

int ZhurinIEdgeSobelSEQ::GradientX(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
        sum += input_pixels_[(ny * width_) + nx] * kSobelX[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

int ZhurinIEdgeSobelSEQ::GradientY(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
        sum += input_pixels_[ny * width_ + nx] * kSobelY[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

}  // namespace zhurin_i_edge_sobel
