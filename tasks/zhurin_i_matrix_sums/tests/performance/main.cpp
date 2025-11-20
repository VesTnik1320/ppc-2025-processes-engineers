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
    std::string input_data_source = ppc::util::GetAbsoluteTaskPath(PPC_ID_zhurin_i_matrix_sums, "perf/input.txt");
    std::string expected_data_source = ppc::util::GetAbsoluteTaskPath(PPC_ID_zhurin_i_matrix_sums, "perf/expected.txt");

    std::ifstream file(input_data_source);
    uint32_t rows = 0;
    uint32_t columns = 0;
    std::vector<double> inp;
    file >> rows;
    file >> columns;
    double num = 0.0;
    while (file >> num) {
      inp.push_back(num);
    }
    file.close();

    file = std::ifstream(expected_data_source);
    double expected_value = 0.0;
    file >> expected_value;
    file.close();

    input_data_ = InType(rows, columns, inp);
    expected_data_ = expected_value;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return std::abs(output_data - expected_data_) < kEpsilon;
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
