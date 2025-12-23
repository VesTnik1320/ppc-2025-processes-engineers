#ifndef ZHURIN_I_EDGE_SOBEL_COMMON_INCLUDE_COMMON_HPP_
#define ZHURIN_I_EDGE_SOBEL_COMMON_INCLUDE_COMMON_HPP_

#include <cstdint>
#include <vector>

namespace zhurin_i_edge_sobel {

struct ImageData {
  std::vector<uint8_t> pixels;  // одномерный массив пикселей в оттенках серого
  int width;                    // ширина изображения
  int height;                   // высота изображения
};

using InType = ImageData;
using OutType = std::vector<uint8_t>;

}  // namespace zhurin_i_edge_sobel

#endif  // ZHURIN_I_EDGE_SOBEL_COMMON_INCLUDE_COMMON_HPP_