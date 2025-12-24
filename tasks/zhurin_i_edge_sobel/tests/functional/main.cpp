#include <gtest/gtest.h>
#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include "zhurin_i_edge_sobel/common/include/common.hpp"
#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"
#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"
#include "util/include/util.hpp"

namespace zhurin_i_sobel_edge {

// Базовый класс для функциональных тестов
template <typename TaskType>
class EdgeDetectionFuncTestsBase : public ppc::util::BaseRunFuncTests<ImageTuple, ResultVector, TestPair> {
public:
    static std::string PrintTestParam(const TestPair &test_data) {
        return std::get<1>(test_data);
    }

protected:
    void SetUp() override {
        auto params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
        ImageSet img_type = std::get<0>(params);
        test_input = prepareInputData(img_type);
        reference_output = fetchExpected(img_type);
    }

    bool CheckTestOutputData(ResultVector &output) final {
        return output == reference_output;
    }

    ImageTuple GetTestInputData() final {
        return test_input;
    }

private:
    ImageTuple test_input;
    ResultVector reference_output;
};

// Конкретные классы для MPI и SEQ
class EdgeDetectionFuncTestsMPI : public EdgeDetectionFuncTestsBase<MPIEdgeProcessor> {};
class EdgeDetectionFuncTestsSEQ : public EdgeDetectionFuncTestsBase<SequentialEdgeDetector> {};

namespace {

// Тесты для MPI
TEST_P(EdgeDetectionFuncTestsMPI, SobelEdgeDetectionTestMPI) {
    ExecuteTest(GetParam());
}

// Тесты для SEQ
TEST_P(EdgeDetectionFuncTestsSEQ, SobelEdgeDetectionTestSEQ) {
    ExecuteTest(GetParam());
}

const std::array<TestPair, 7> test_cases = {
    std::make_tuple(ImageSet::SAMPLE_1, "basic_5x5_matrix"),
    std::make_tuple(ImageSet::SAMPLE_2, "random_10x10_data"),
    std::make_tuple(ImageSet::SAMPLE_3, "large_dataset_test"),
    std::make_tuple(ImageSet::SAMPLE_4, "rectangular_5x10"),
    std::make_tuple(ImageSet::SAMPLE_5, "rectangular_10x5"),
    std::make_tuple(ImageSet::SAMPLE_6, "kernel_edge_3x3"),
    std::make_tuple(ImageSet::SAMPLE_7, "minimal_2x2_test")
};

// Отдельные конфигурации для MPI и SEQ
const auto test_configs_mpi = ppc::util::AddFuncTask<MPIEdgeProcessor, ImageTuple>(
        test_cases, PPC_SETTINGS_zhurin_i_edge_sobel);
const auto test_configs_seq = ppc::util::AddFuncTask<SequentialEdgeDetector, ImageTuple>(
        test_cases, PPC_SETTINGS_zhurin_i_edge_sobel);

const auto gtest_params_mpi = ppc::util::ExpandToValues(test_configs_mpi);
const auto gtest_params_seq = ppc::util::ExpandToValues(test_configs_seq);

// Отдельные инстансы для MPI и SEQ
const auto test_naming_mpi = EdgeDetectionFuncTestsMPI::PrintFuncTestName<EdgeDetectionFuncTestsMPI>;
const auto test_naming_seq = EdgeDetectionFuncTestsSEQ::PrintFuncTestName<EdgeDetectionFuncTestsSEQ>;

INSTANTIATE_TEST_SUITE_P(PicMatrixTestsMPI, EdgeDetectionFuncTestsMPI, gtest_params_mpi, test_naming_mpi);
INSTANTIATE_TEST_SUITE_P(PicMatrixTestsSEQ, EdgeDetectionFuncTestsSEQ, gtest_params_seq, test_naming_seq);

} // namespace
} // namespace zhurin_i_sobel_edge