#ifndef ZHURIN_I_EDGE_DETECTION_MPI_HPP
#define ZHURIN_I_EDGE_DETECTION_MPI_HPP

#include <vector>

#include "task/include/task.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_sobel_edge {

class MPIEdgeProcessor : public TaskInterface {
 public:
  static constexpr auto getTypeMarker() {
    return ppc::task::TypeOfTask::kMPI;
  }

  explicit MPIEdgeProcessor(const ImageTuple &input);

 private:
  int rows, cols, cutoff;
  std::vector<int> sourceImage;
  std::vector<int> chunk;
  int chunkRows, chunkRowsWithBorder;

  void broadcastMetadata();
  void splitImage();
  std::vector<int> processChunk();
  int calcGradH(int px, int py);
  int calcGradV(int px, int py);
  void assembleResults(const std::vector<int> &partial);

  // Гало-строки - без изменений
  void RowDistributionComputing(int world_rank, int world_size, int &base_rows, int &remainder, int &real_rows,
                                int &need_top_halo, int &need_bottom_halo, int &total_rows);
  void SendParameters(int world_rank, int world_size, int base_rows, int remainder,
                      std::vector<int> &real_rows_per_proc, std::vector<int> &send_counts,
                      std::vector<int> &send_displs) const;
  void DataDistribution(int world_rank, const std::vector<int> &send_counts, const std::vector<int> &send_displs);

  bool ValidationImpl() final;
  bool PreProcessingImpl() final;
  bool RunImpl() final;
  bool PostProcessingImpl() final;
};

}  // namespace zhurin_i_sobel_edge

#endif
