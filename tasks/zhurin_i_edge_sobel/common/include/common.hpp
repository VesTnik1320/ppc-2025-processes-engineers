#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace zhurin_i_edge_sobel {

enum class ImageSet : std::uint8_t { kTest1, kTest2, kTest3, kTest4, kTest5, kTest6, kTest7, kTest8, kTest9, kTest10 };

using InType = std::tuple<std::vector<int>, int, int, int>;
using OutType = std::vector<int>;
using TestType = std::tuple<ImageSet, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

inline std::tuple<std::vector<int>, int, int> ReadImageFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return std::make_tuple(std::vector<int>(), 0, 0);
  }

  int height = 0;
  int width = 0;

  file >> height;
  file >> width;

  std::vector<int> pixels;
  pixels.reserve(static_cast<size_t>(height) * width);

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      int pixel = 0;
      if (!(file >> pixel)) {
        return std::make_tuple(std::vector<int>(), 0, 0);
      }
      pixels.push_back(pixel);
    }
  }

  file.close();
  return std::make_tuple(pixels, height, width);
}

inline std::string GetDirectoryPath(const std::string &full_path) {
  std::string result = full_path;
  const std::string json_part = "settings.json";

  if (result.size() >= json_part.size()) {
    size_t pos = result.size() - json_part.size();
    if (result.compare(pos, json_part.size(), json_part) == 0) {
      result.erase(pos, json_part.size());
    }
  }

  return result;
}

inline std::string GetTestFilename(ImageSet type, const std::string &folder) {
  int test_num = 0;
  switch (type) {
    case ImageSet::kTest1:
      test_num = 1;
      break;
    case ImageSet::kTest2:
      test_num = 2;
      break;
    case ImageSet::kTest3:
      test_num = 3;
      break;
    case ImageSet::kTest4:
      test_num = 4;
      break;
    case ImageSet::kTest5:
      test_num = 5;
      break;
    case ImageSet::kTest6:
      test_num = 6;
      break;
    case ImageSet::kTest7:
      test_num = 7;
      break;
    case ImageSet::kTest8:
      test_num = 8;
      break;
    case ImageSet::kTest9:
      test_num = 9;
      break;
    case ImageSet::kTest10:
      test_num = 10;
      break;
    default:
      test_num = 1;
  }

  return folder + "/test" + std::to_string(test_num) + ".txt";
}

inline std::string GetTestFilename(ImageSet type) {
  int test_num = static_cast<int>(type) + 1;  // kTest1 -> 1, kTest2 -> 2 и т.д.
  return "test" + std::to_string(test_num) + ".txt";
}

inline std::tuple<std::vector<int>, int, int, int> GenerateTestData(ImageSet type) {
  std::string full_path = GetDirectoryPath(PPC_SETTINGS_zhurin_i_edge_sobel) + "tasks/zhurin_i_edge_sobel/data/cases/" +
                          GetTestFilename(type);

  std::tuple<std::vector<int>, int, int> read_result = ReadImageFile(full_path);

  std::vector<int> pixels = std::get<0>(read_result);
  int height = std::get<1>(read_result);
  int width = std::get<2>(read_result);

  int threshold = 100;

  return std::make_tuple(pixels, height, width, threshold);
}

inline std::vector<int> GenerateExpectedOutput(ImageSet type) {
  std::string full_path = GetDirectoryPath(PPC_SETTINGS_zhurin_i_edge_sobel) +
                          "tasks/zhurin_i_edge_sobel/data/expected/" + GetTestFilename(type);

  std::tuple<std::vector<int>, int, int> read_result = ReadImageFile(full_path);

  return std::get<0>(read_result);
}

}  // namespace zhurin_i_edge_sobel
