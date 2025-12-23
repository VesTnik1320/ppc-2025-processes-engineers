#include <gtest/gtest.h>
#include <mpi.h>

#include <vector>
#include <tuple>
#include <algorithm>
#include <string>

#include "task/include/task.hpp"
#include "util/include/func_test_util.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"
#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"
#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

namespace zhurin_i_edge_sobel {

using TestType = std::tuple<int, ImageData>;

class EdgeSobelFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestName(
      const testing::TestParamInfo<std::tuple<std::function<std::shared_ptr<ppc::task::Task<InType, OutType>>(InType)>,
                                              std::string, TestType>> &info) {
    return std::get<1>(info.param) + "_Test" + std::to_string(std::get<0>(std::get<2>(info.param)));
  }

 protected:
  void SetUp() override {
    const auto &params = std::get<2>(GetParam());
    test_input_ = std::get<1>(params);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != static_cast<size_t>(test_input_.width * test_input_.height)) {
      return false;
    }
    
    for (auto pixel : output_data) {
      if (pixel > 255) return false;
    }
    
    return true;
  }

  InType GetTestInputData() final {
    return test_input_;
  }

 private:
  ImageData test_input_;
};

namespace {

// Создаем простые тестовые изображения
std::vector<uint8_t> CreateBlackImage(int w, int h) {
  return std::vector<uint8_t>(w * h, 0);
}

std::vector<uint8_t> CreateWhiteImage(int w, int h) {
  return std::vector<uint8_t>(w * h, 255);
}

std::vector<uint8_t> CreateGradientImage(int w, int h) {
  std::vector<uint8_t> img(w * h);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      img[y * w + x] = static_cast<uint8_t>((x + y) * 255 / (w + h));
    }
  }
  return img;
}

std::vector<uint8_t> CreateStripesImage(int w, int h) {
  std::vector<uint8_t> img(w * h);
  for (int y = 0; y < h; ++y) {
    for (int x = 0; x < w; ++x) {
      img[y * w + x] = (x % 4 < 2) ? 100 : 200;
    }
  }
  return img;
}

std::vector<uint8_t> CreateDotImage(int w, int h) {
  std::vector<uint8_t> img(w * h, 50);
  int center_x = w / 2;
  int center_y = h / 2;
  if (center_x < w && center_y < h) {
    img[center_y * w + center_x] = 250;
  }
  return img;
}

std::vector<uint8_t> CreateCrossImage(int w, int h) {
  std::vector<uint8_t> img(w * h, 80);
  // Вертикальная линия
  for (int y = 0; y < h; ++y) {
    int x = w / 2;
    img[y * w + x] = 200;
  }
  // Горизонтальная линия
  for (int x = 0; x < w; ++x) {
    int y = h / 2;
    img[y * w + x] = 200;
  }
  return img;
}

// 12 простых тестов
const std::array<TestType, 12> kAllTests = {
    // Минимальные размеры
    std::make_tuple(1, ImageData{CreateBlackImage(3, 3), 3, 3}),
    std::make_tuple(2, ImageData{CreateWhiteImage(3, 3), 3, 3}),
    
    // Разные размеры
    std::make_tuple(3, ImageData{CreateGradientImage(5, 5), 5, 5}),
    std::make_tuple(4, ImageData{CreateStripesImage(7, 7), 7, 7}),
    
    // Прямоугольные
    std::make_tuple(5, ImageData{CreateGradientImage(8, 4), 8, 4}),
    std::make_tuple(6, ImageData{CreateStripesImage(4, 8), 4, 8}),
    
    // Большие
    std::make_tuple(7, ImageData{CreateBlackImage(10, 10), 10, 10}),
    std::make_tuple(8, ImageData{CreateWhiteImage(10, 10), 10, 10}),
    
    // С деталями
    std::make_tuple(9, ImageData{CreateDotImage(9, 9), 9, 9}),
    std::make_tuple(10, ImageData{CreateCrossImage(9, 9), 9, 9}),
    
    // Для MPI распределения
    std::make_tuple(11, ImageData{CreateGradientImage(12, 8), 12, 8}),
    std::make_tuple(12, ImageData{CreateStripesImage(16, 16), 16, 16}),
};

const auto kAllTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<zhurin_i_edge_sobel::ZhurinIEdgeSobelMPI, InType>(
                       kAllTests, PPC_SETTINGS_zhurin_i_edge_sobel),
                   ppc::util::AddFuncTask<zhurin_i_edge_sobel::ZhurinIEdgeSobelSEQ, InType>(
                       kAllTests, PPC_SETTINGS_zhurin_i_edge_sobel));

inline const auto kGtestValues = ppc::util::ExpandToValues(kAllTasksList);

TEST_P(EdgeSobelFuncTests, BasicEdgeDetection) {
  ExecuteTest(GetParam());
}

INSTANTIATE_TEST_SUITE_P(EdgeSobelSuite, EdgeSobelFuncTests, kGtestValues,
                         EdgeSobelFuncTests::PrintTestName);

}  // namespace

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  
  MPI_Init(&argc, &argv);
  
  int result = RUN_ALL_TESTS();
  
  MPI_Finalize();
  return result;
}

}  // namespace zhurin_i_edge_sobel