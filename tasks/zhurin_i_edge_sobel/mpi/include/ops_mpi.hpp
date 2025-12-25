#pragma once

// #include <mpi.h>

#include <cmath>
#include <cstddef>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

// Собелевские ядра
const std::vector<std::vector<int>> kSobelX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const std::vector<std::vector<int>> kSobelY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

class ZhurinIEdgeSobelMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit ZhurinIEdgeSobelMPI(const InType &in);

 private:
  int height_ = 0;
  int width_ = 0;
  int threshold_ = 0;

  std::vector<int> input_pixels_;
  std::vector<int> local_pixels_;
  int local_height_ = 0;
  int local_height_with_halo_ = 0;

  void BroadcastParameters();
  void DistributeRows();
  std::vector<int> LocalGradientsComputing();
  int GradientX(int x, int y) const;
  int GradientY(int x, int y) const;
  void GatherResults(const std::vector<int> &local_result);

  void RowDistributionComputing(int world_rank, int world_size, int &base_rows, int &remainder, int &real_rows,
                                int &need_top_halo, int &need_bottom_halo, int &total_rows);

  void SendParameters(int world_rank, int world_size, int base_rows, int remainder,
                      std::vector<int> &real_rows_per_proc, std::vector<int> &send_counts,
                      std::vector<int> &send_displs) const;

  void LocalRowsComputing(int /*unused*/, int /*unused*/);
  void DataDistribution(int /*unused*/, const std::vector<int> & /*unused*/, const std::vector<int> & /*unused*/);

  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zhurin_i_edge_sobel
