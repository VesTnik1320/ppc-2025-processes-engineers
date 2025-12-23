#ifndef ZHURIN_I_EDGE_SOBEL_MPI_INCLUDE_OPS_MPI_HPP_
#define ZHURIN_I_EDGE_SOBEL_MPI_INCLUDE_OPS_MPI_HPP_

#include <vector>
#include <cstdint>
#include <string>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

class ZhurinIEdgeSobelMPI {
 public:
  explicit ZhurinIEdgeSobelMPI(const InType& in);
  
  static std::string GetStaticTypeOfTask();
  std::string GetTypeOfTask() const;
  
  const InType& GetInput() const;
  InType& GetInput();
  const OutType& GetOutput() const;
  OutType& GetOutput();

  bool Validation();
  bool PreProcessing();
  bool Run();
  bool PostProcessing();

 private:
  void SetTypeOfTask(const std::string& type);
  
  // MPI функции
  void BroadcastParameters();
  void DistributeData();
  void ExchangeBoundaries(std::vector<uint8_t>& local_data);
  std::vector<uint8_t> ProcessLocalData(const std::vector<uint8_t>& local_data);
  void GatherResults(const std::vector<uint8_t>& local_result);
  
  InType input_;
  OutType output_;
  std::string task_type_;
  
  // MPI переменные
  int world_rank_;
  int world_size_;
  int local_width_;
  int local_height_;
  int local_start_row_;
  int rows_per_process_;
  int rows_with_boundaries_;
};

}  // namespace zhurin_i_edge_sobel

#endif  // ZHURIN_I_EDGE_SOBEL_MPI_INCLUDE_OPS_MPI_HPP_