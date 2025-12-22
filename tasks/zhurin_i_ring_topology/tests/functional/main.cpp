#include <gtest/gtest.h>
#include <mpi.h>

#include <algorithm>
#include <array>
#include <climits>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"
#include "util/include/func_test_util.hpp"
#include "zhurin_i_ring_topology/common/include/common.hpp"
#include "zhurin_i_ring_topology/mpi/include/ops_mpi.hpp"
#include "zhurin_i_ring_topology/seq/include/ops_seq.hpp"

namespace zhurin_i_ring_topology {

using TestType = std::tuple<int, RingMessage>;

class ZhurinIRingTopologyFuncTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 public:
  static std::string PrintTestName(
      const testing::TestParamInfo<std::tuple<std::function<std::shared_ptr<ppc::task::Task<InType, OutType>>(InType)>,
                                              std::string, TestType>> &info) {
    const auto &task_name = std::get<1>(info.param);
    const auto &test_param = std::get<2>(info.param);
    const int test_id = std::get<0>(test_param);
    return task_name + "Test" + std::to_string(test_id);
  }

 protected:
  void SetUp() override {
    const auto &params = std::get<2>(GetParam());
    test_message_ = std::get<1>(params);

    int is_mpi_initialized = 0;
    MPI_Initialized(&is_mpi_initialized);
    if (is_mpi_initialized != 0) {
      int size = 0;
      MPI_Comm_size(MPI_COMM_WORLD, &size);
      const int max_node = std::max(test_message_.source, test_message_.dest);
      const int required_size = std::max(size, max_node + 1);
      if (size < required_size) {
        GTEST_SKIP() << "Test requires at least " << required_size << " processes, but only " << size << " available.";
      }
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return output_data == test_message_.data;
  }

  InType GetTestInputData() final {
    return test_message_;
  }

 private:
  RingMessage test_message_;
};

namespace {

const std::array<TestType, 28> kAllTests = {
    std::make_tuple(1, RingMessage{.source = 0, .dest = 2, .data = {1, 3, 5, 7, 9}, .go_clockwise = true}),
    std::make_tuple(2, RingMessage{.source = 7, .dest = 1, .data = {111, 222, 333, 444}, .go_clockwise = true}),
    std::make_tuple(3, RingMessage{.source = 4, .dest = 4, .data = {1, 2, 3, 4, 5}, .go_clockwise = true}),
    std::make_tuple(4, RingMessage{.source = 0, .dest = 6, .data = {42, 52, 62, 72}, .go_clockwise = true}),
    std::make_tuple(5, RingMessage{.source = 2, .dest = 5, .data = {}, .go_clockwise = true}),
    std::make_tuple(6, RingMessage{.source = 3, .dest = 7, .data = {999}, .go_clockwise = true}),
    std::make_tuple(7, RingMessage{.source = 1, .dest = 1, .data = {111}, .go_clockwise = true}),
    std::make_tuple(8, RingMessage{.source = 0, .dest = 3, .data = {-1, -2, -3}, .go_clockwise = true}),
    std::make_tuple(9, RingMessage{.source = 5, .dest = 2, .data = {0, 0, 0, 0}, .go_clockwise = true}),
    std::make_tuple(10, RingMessage{.source = 0, .dest = 1, .data = {INT_MAX, INT_MIN}, .go_clockwise = true}),
    std::make_tuple(11, RingMessage{.source = 6, .dest = 0, .data = {1, 2, 3, 4, 5, 6, 7}, .go_clockwise = true}),
    std::make_tuple(12, RingMessage{.source = 2, .dest = 2, .data = std::vector<int>(100, 52), .go_clockwise = true}),
    std::make_tuple(13, RingMessage{.source = 0, .dest = 4, .data = std::vector<int>(50, 5), .go_clockwise = true}),
    std::make_tuple(14, RingMessage{.source = 4, .dest = 0, .data = {1, -2, 3, -4, 5, 6}, .go_clockwise = true}),
    std::make_tuple(15, RingMessage{.source = 0, .dest = 7, .data = {1, 2, 3, 4, 5, 6, 7, 8}, .go_clockwise = true}),
    std::make_tuple(16, RingMessage{.source = 7, .dest = 0, .data = {8, 7, 6, 5, 4, 3, 2, 1}, .go_clockwise = true}),
    std::make_tuple(17, RingMessage{.source = 0, .dest = 0, .data = {1, 2, 3}, .go_clockwise = true}),
    std::make_tuple(18, RingMessage{.source = 0, .dest = 0, .data = {7}, .go_clockwise = true}),
    std::make_tuple(19, RingMessage{.source = 0, .dest = 0, .data = {-1, -2, -3}, .go_clockwise = true}),
    std::make_tuple(20, RingMessage{.source = 0, .dest = 0, .data = {0, 0, 0}, .go_clockwise = true}),

    std::make_tuple(21, RingMessage{.source = 0, .dest = 2, .data = {1, 3, 5, 7, 9}, .go_clockwise = false}),
    std::make_tuple(22, RingMessage{.source = 7, .dest = 1, .data = {111, 222, 333, 444}, .go_clockwise = false}),
    std::make_tuple(23, RingMessage{.source = 0, .dest = 6, .data = {42, 52, 62, 72}, .go_clockwise = false}),
    std::make_tuple(24, RingMessage{.source = 2, .dest = 5, .data = {}, .go_clockwise = false}),
    std::make_tuple(25, RingMessage{.source = 3, .dest = 7, .data = {999}, .go_clockwise = false}),
    std::make_tuple(26, RingMessage{.source = 0, .dest = 3, .data = {-1, -2, -3}, .go_clockwise = false}),
    std::make_tuple(27, RingMessage{.source = 5, .dest = 2, .data = {0, 0, 0, 0}, .go_clockwise = false}),
    std::make_tuple(28, RingMessage{.source = 6, .dest = 0, .data = {1, 2, 3, 4, 5, 6, 7}, .go_clockwise = false}),
};

const auto kAllTasksList =
    std::tuple_cat(ppc::util::AddFuncTask<zhurin_i_ring_topology::ZhurinIRingTopologyMPI, InType>(
                       kAllTests, PPC_SETTINGS_zhurin_i_ring_topology),
                   ppc::util::AddFuncTask<zhurin_i_ring_topology::ZhurinIRingTopologySEQ, InType>(
                       kAllTests, PPC_SETTINGS_zhurin_i_ring_topology));

inline const auto kGtestValues = ppc::util::ExpandToValues(kAllTasksList);

TEST_P(ZhurinIRingTopologyFuncTests, AllTests) {
  ExecuteTest(GetParam());
}

INSTANTIATE_TEST_SUITE_P(ZhurinIRingTopology, ZhurinIRingTopologyFuncTests, kGtestValues,
                         ZhurinIRingTopologyFuncTests::PrintTestName);

}  // namespace

}  // namespace zhurin_i_ring_topology
