#include <gtest/gtest.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "zhurin_i_ring_topology/common/include/common.hpp"
#include "zhurin_i_ring_topology/mpi/include/ops_mpi.hpp"
#include "zhurin_i_ring_topology/seq/include/ops_seq.hpp"

namespace zhurin_i_ring_topology {

class ZhurinIRingTopologyFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return std::to_string(std::get<0>(test_param));
  }

 protected:
  void SetUp() override {
    TestType param = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    int test_id = std::get<0>(param);
    std::string input_data_source =
        ppc::util::GetAbsoluteTaskPath(PPC_ID_zhurin_i_ring_topology, "cases/test" + std::to_string(test_id) + ".txt");
    std::string expected_data_source =
        ppc::util::GetAbsoluteTaskPath(PPC_ID_zhurin_i_ring_topology, "expected/test" + std::to_string(test_id) + ".txt");

    // Чтение входных данных
    std::ifstream file(input_data_source);
    int source = 0, dest = 0;
    std::vector<int> data;
    
    if (file.is_open()) {
      file >> source >> dest;
      int value;
      while (file >> value) {
        data.push_back(value);
      }
      file.close();
    } else {
      FAIL() << "Cannot open input file: " << input_data_source;
    }

    std::vector<int> expected_data;
    file.open(expected_data_source);
    if (file.is_open()) {
      int value;
      while (file >> value) {
        expected_data.push_back(value);
      }
      file.close();
    }

    input_data_ = {source, dest, data};
    expected_data_ = expected_data;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    if (output_data.size() != expected_data_.size()) {
      return false;
    }
    
    for (size_t i = 0; i < output_data.size(); ++i) {
      if (output_data[i] != expected_data_[i]) {
        return false;
      }
    }
    return true;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_data_;
};

namespace {

TEST_P(ZhurinIRingTopologyFuncTests, RingTopology) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 6> kTestParam = {
    std::make_tuple(1, RingMessage{}),
    std::make_tuple(2, RingMessage{}),
    std::make_tuple(3, RingMessage{}),
    std::make_tuple(4, RingMessage{}),
    std::make_tuple(5, RingMessage{}),
    std::make_tuple(6, RingMessage{})
};

const auto kTestTasksList = std::tuple_cat(
    ppc::util::AddFuncTask<zhurin_i_ring_topology::ZhurinIRingTopologyMPI, InType>(
        kTestParam, PPC_SETTINGS_zhurin_i_ring_topology),
    ppc::util::AddFuncTask<zhurin_i_ring_topology::ZhurinIRingTopologySEQ, InType>(
        kTestParam, PPC_SETTINGS_zhurin_i_ring_topology));

inline const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

inline const auto kPerfTestName = ZhurinIRingTopologyFuncTests::PrintFuncTestName<ZhurinIRingTopologyFuncTests>;

// NOLINTNEXTLINE
INSTANTIATE_TEST_SUITE_P(ZhurinIRingTopology, ZhurinIRingTopologyFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zhurin_i_ring_topology
