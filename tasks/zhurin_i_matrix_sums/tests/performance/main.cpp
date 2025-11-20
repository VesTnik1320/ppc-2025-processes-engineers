#include <gtest/gtest.h>

#include <cmath>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "util/include/perf_test_util.hpp"
#include "util/include/util.hpp"
#include "zhurin_i_matrix_sums/common/include/common.hpp"
#include "zhurin_i_matrix_sums/mpi/include/ops_mpi.hpp"
#include "zhurin_i_matrix_sums/seq/include/ops_seq.hpp"

namespace zhurin_i_matrix_sums {

class ZhurinIMatrixSumsPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
  InType input_data_{0, 0, {}};
  OutType expected_data_{0.0};

  void SetUp() override {
    // Генерируем матрицу программно вместо чтения из файла
    uint32_t rows = 10000;  // Можно увеличить до любого размера
    uint32_t columns = 10000;
    uint64_t totalElements = static_cast<uint64_t>(rows) * columns;

    std::vector<double> inp(totalElements);

    // Заполняем матрицу числами от 1 до totalElements
    for (uint64_t i = 0; i < totalElements; ++i) {
      inp[i] = static_cast<double>(i + 1);
    }

    // Вычисляем ожидаемую сумму (формула суммы арифметической прогрессии)
    double expected_value = static_cast<double>(totalElements) * (totalElements + 1) / 2;

    input_data_ = InType(rows, columns, inp);
    expected_data_ = expected_value;

    std::cout << "PERF TEST: Generated matrix " << rows << "x" << columns << " (" << totalElements << " elements)"
              << std::endl;
    std::cout << "PERF TEST: Expected sum = " << expected_value << std::endl;
  }
  bool CheckTestOutputData(OutType &output_data) final {
    if (expected_data_ == 0.0) {
      return output_data == 0.0;
    }
    double relative_error = std::abs(output_data - expected_data_) / std::abs(expected_data_);
    return relative_error < 1e-10;
  }

  [[nodiscard]] InType GetTestInputData() final {
    return input_data_;
  }
};

namespace {

TEST_P(ZhurinIMatrixSumsPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, ZhurinIMatrixSumsMPI, ZhurinIMatrixSumsSEQ>(PPC_SETTINGS_zhurin_i_matrix_sums);

inline const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

inline const auto kPerfTestName = ZhurinIMatrixSumsPerfTests::CustomPerfTestName;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(ZhurinPerf, ZhurinIMatrixSumsPerfTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zhurin_i_matrix_sums
