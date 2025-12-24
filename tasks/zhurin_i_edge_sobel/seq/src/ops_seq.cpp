#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

#include <array>
#include <cmath>
#include <vector>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_sobel_edge {

static constexpr std::array<std::array<int, 3>, 3> H_FILTER = {
    std::array<int, 3>{-1, 0, 1}, std::array<int, 3>{-2, 0, 2}, std::array<int, 3>{-1, 0, 1}};

static constexpr std::array<std::array<int, 3>, 3> V_FILTER = {
    std::array<int, 3>{-1, -2, -1}, std::array<int, 3>{0, 0, 0}, std::array<int, 3>{1, 2, 1}};

SequentialEdgeDetector::SequentialEdgeDetector(const ImageTuple &input)
    : height(std::get<1>(input)), width(std::get<2>(input)), limit(std::get<3>(input)) {
  SetTypeOfTask(getTypeMarker());
  GetInput() = input;
}

bool SequentialEdgeDetector::ValidationImpl() {
  return height > 0 && width > 0 && limit >= 0;
}

bool SequentialEdgeDetector::PreProcessingImpl() {
  GetOutput() = std::vector<int>(static_cast<size_t>(height * width), 0);
  return true;
}

bool SequentialEdgeDetector::RunImpl() {
  pixels = std::get<0>(GetInput());
  auto &output = GetOutput();

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      int h = computeHorizontal(x, y);
      int v = computeVertical(x, y);

      int magnitude = static_cast<int>(std::sqrt(h * h + v * v));
      output[y * width + x] = magnitude > limit ? magnitude : 0;
    }
  }

  return true;
}

bool SequentialEdgeDetector::PostProcessingImpl() {
  return true;
}

int SequentialEdgeDetector::computeHorizontal(int x, int y) {
  int sum = 0;

  for (int dy = -1; dy <= 1; ++dy) {
    int ny = y + dy;
    if (ny < 0 || ny >= height) {
      continue;
    }

    for (int dx = -1; dx <= 1; ++dx) {
      int nx = x + dx;
      if (nx < 0 || nx >= width) {
        continue;
      }

      sum += pixels[ny * width + nx] * H_FILTER[dy + 1][dx + 1];
    }
  }

  return sum;
}

int SequentialEdgeDetector::computeVertical(int x, int y) {
  int sum = 0;

  for (int dy = -1; dy <= 1; ++dy) {
    int ny = y + dy;
    if (ny < 0 || ny >= height) {
      continue;
    }

    for (int dx = -1; dx <= 1; ++dx) {
      int nx = x + dx;
      if (nx < 0 || nx >= width) {
        continue;
      }

      sum += pixels[ny * width + nx] * V_FILTER[dy + 1][dx + 1];
    }
  }

  return sum;
}

}  // namespace zhurin_i_sobel_edge
