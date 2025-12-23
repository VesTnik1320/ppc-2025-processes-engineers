#ifndef ZHURIN_I_EDGE_SOBEL_PERF_TESTS_HPP_
#define ZHURIN_I_EDGE_SOBEL_PERF_TESTS_HPP_

#include <gtest/gtest.h>

#include <cstddef>
#include <random>
#include <vector>

#include "util/include/perf_test_util.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"
#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"
#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

namespace zhurin_i_edge_sobel {

class ZhurinIEdgeSobelPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    // Создаем одно большое изображение 2000x2000 для теста производительности
    const int width = 2000;
    const int height = 2000;
    
    input_data_.width = width;
    input_data_.height = height;
    input_data_.pixels.resize(static_cast<size_t>(width) * height);
    
    // Используем простой детерминированный паттерн
    std::mt19937 rng(42);  // Фиксированный seed для воспроизводимости
    std::uniform_int_distribution<int> dist(0, 255);
    
    // Создаем изображение с вертикальными и горизонтальными полосами
    // Это создаст много границ для детектирования
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        // Вертикальные полосы шириной 100 пикселей
        bool vertical_stripe = (x / 100) % 2 == 0;
        // Горизонтальные полосы шириной 80 пикселей
        bool horizontal_stripe = (y / 80) % 2 == 0;
        
        int base_value = 128;
        if (vertical_stripe) base_value += 50;
        if (horizontal_stripe) base_value += 30;
        
        // Добавляем небольшой шум
        int noise = dist(rng) % 20 - 10;
        int final_value = base_value + noise;
        
        // Ограничиваем диапазон
        if (final_value < 0) final_value = 0;
        if (final_value > 255) final_value = 255;
        
        input_data_.pixels[static_cast<size_t>(y) * width + x] = 
            static_cast<uint8_t>(final_value);
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    // Проверяем базовую корректность результата
    if (output_data.empty()) {
      return false;
    }
    
    size_t expected_size = static_cast<size_t>(input_data_.width * input_data_.height);
    if (output_data.size() != expected_size) {
      return false;
    }
    
    // Для изображения с полосами должны быть детектированы границы
    // Проверим, что есть хотя бы 0.1% ненулевых значений (границ)
    size_t non_zero_count = 0;
    for (auto pixel : output_data) {
      if (pixel > 20) {  // Порог для значимой границы
        non_zero_count++;
      }
    }
    
    // Ожидаем хотя бы 0.1% границ
    size_t min_expected = expected_size / 1000;
    return non_zero_count > min_expected;
  }

  [[nodiscard]] InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
};

namespace {

TEST_P(ZhurinIEdgeSobelPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks = ppc::util::MakeAllPerfTasks<InType, ZhurinIEdgeSobelMPI, ZhurinIEdgeSobelSEQ>(
    PPC_SETTINGS_zhurin_i_edge_sobel);

inline const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

inline const auto kPerfTestName = ZhurinIEdgeSobelPerfTests::CustomPerfTestName;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(ZhurinIEdgeSobelPerf, ZhurinIEdgeSobelPerfTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zhurin_i_edge_sobel

#endif  // ZHURIN_I_EDGE_SOBEL_PERF_TESTS_HPP_