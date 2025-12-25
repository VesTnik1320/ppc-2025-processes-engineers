#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>

#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"
#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"
#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

namespace zhurin_i_edge_sobel {

class EdgeSobelFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestParam(const TestType &test_param) {
    std::string description = std::get<1>(test_param);
    return description;
  }

 protected:
  void SetUp() override {
    auto params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    ImageSet image_type = std::get<0>(params);
    input_data_ = GenerateTestData(image_type);
    expected_output_ = GenerateExpectedOutput(image_type);
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == expected_output_;
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_;
  OutType expected_output_;
};

namespace {

TEST_P(EdgeSobelFuncTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 10> kTestParams = {
    std::make_tuple(ImageSet::kTest1, "test1"), std::make_tuple(ImageSet::kTest2, "test2"),
    std::make_tuple(ImageSet::kTest3, "test3"), std::make_tuple(ImageSet::kTest4, "test4"),
    std::make_tuple(ImageSet::kTest5, "test5"), std::make_tuple(ImageSet::kTest6, "test6"),
    std::make_tuple(ImageSet::kTest7, "test7"), std::make_tuple(ImageSet::kTest8, "test8"),
    std::make_tuple(ImageSet::kTest9, "test9"), std::make_tuple(ImageSet::kTest10, "test10")};

const auto kTestTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<ZhurinIEdgeSobelMPI, InType>(kTestParams, PPC_SETTINGS_zhurin_i_edge_sobel),
                   ppc::util::AddFuncTask<ZhurinIEdgeSobelSEQ, InType>(kTestParams, PPC_SETTINGS_zhurin_i_edge_sobel));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);

const auto kPerfTestName = EdgeSobelFuncTests::PrintFuncTestName<EdgeSobelFuncTests>;

INSTANTIATE_TEST_SUITE_P(RunMatrixTests, EdgeSobelFuncTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace zhurin_i_edge_sobel
