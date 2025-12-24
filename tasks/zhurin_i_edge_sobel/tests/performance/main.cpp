#include <gtest/gtest.h>
#include <algorithm>
#include <tuple>
#include <vector>
#include "zhurin_i_edge_sobel/common/include/common.hpp"
#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"
#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace zhurin_i_sobel_edge {

// Базовый класс для производительных тестов
template <typename TaskType>
class EdgeDetectionPerfTestsBase : public ppc::util::BaseRunPerfTests<ImageTuple, ResultVector> {
    ImageTuple test_data_;
    ResultVector expected_result_;

    void SetUp() override {
        const int dimension = 2000;
        const int rows = dimension;
        const int cols = dimension;
        
        std::vector<int> pixel_buffer(rows * cols);
        
        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                int pixel_val = ((i * 73) + (j * 97)) % 113;
                pixel_buffer[i * cols + j] = pixel_val;
            }
        }
        
        for (int i = 0; i < rows; ++i) {
            int center_col = cols / 2;
            pixel_buffer[i * cols + center_col] = 255;
            if (center_col + 1 < cols) {
                pixel_buffer[i * cols + center_col + 1] = 255;
            }
        }
        
        for (int j = 0; j < cols; ++j) {
            int center_row = rows / 3;
            pixel_buffer[center_row * cols + j] = 255;
            if (center_row + 1 < rows) {
                pixel_buffer[(center_row + 1) * cols + j] = 255;
            }
        }
        
        int diag_size = std::min(rows, cols);
        for (int k = 0; k < diag_size; ++k) {
            pixel_buffer[k * cols + k] = 255;
            if (k + 1 < cols) {
                pixel_buffer[k * cols + (k + 1)] = 255;
            }
        }
        
        const int edge_thresh = 100;
        test_data_ = std::make_tuple(pixel_buffer, rows, cols, edge_thresh);
        expected_result_ = std::vector<int>(pixel_buffer.size(), 0);
    }

    bool CheckTestOutputData(ResultVector &output) final {
        return output.size() == expected_result_.size();
    }

    ImageTuple GetTestInputData() final {
        return test_data_;
    }
};

// Конкретные классы для MPI и SEQ
class EdgeDetectionPerfTestsMPI : public EdgeDetectionPerfTestsBase<MPIEdgeProcessor> {};
class EdgeDetectionPerfTestsSEQ : public EdgeDetectionPerfTestsBase<SequentialEdgeDetector> {};

TEST_P(EdgeDetectionPerfTestsMPI, PerformanceBenchmarkMPI) {
    ExecuteTest(GetParam());
}

TEST_P(EdgeDetectionPerfTestsSEQ, PerformanceBenchmarkSEQ) {
    ExecuteTest(GetParam());
}

// Конфигурации для MPI и SEQ
const auto perf_test_configs_mpi = ppc::util::MakeAllPerfTasks<ImageTuple, MPIEdgeProcessor>(
    PPC_SETTINGS_zhurin_i_edge_sobel);
const auto perf_test_configs_seq = ppc::util::MakeAllPerfTasks<ImageTuple, SequentialEdgeDetector>(
    PPC_SETTINGS_zhurin_i_edge_sobel);

const auto gtest_perf_values_mpi = ppc::util::TupleToGTestValues(perf_test_configs_mpi);
const auto gtest_perf_values_seq = ppc::util::TupleToGTestValues(perf_test_configs_seq);

const auto perf_test_naming_mpi = EdgeDetectionPerfTestsMPI::CustomPerfTestName;
const auto perf_test_naming_seq = EdgeDetectionPerfTestsSEQ::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTestsMPI, EdgeDetectionPerfTestsMPI, gtest_perf_values_mpi, perf_test_naming_mpi);
INSTANTIATE_TEST_SUITE_P(RunModeTestsSEQ, EdgeDetectionPerfTestsSEQ, gtest_perf_values_seq, perf_test_naming_seq);

} // namespace zhurin_i_sobel_edge