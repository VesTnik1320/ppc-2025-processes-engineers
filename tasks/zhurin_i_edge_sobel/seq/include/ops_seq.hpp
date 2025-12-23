#ifndef ZHURIN_I_EDGE_SOBEL_SEQ_INCLUDE_OPS_SEQ_HPP_
#define ZHURIN_I_EDGE_SOBEL_SEQ_INCLUDE_OPS_SEQ_HPP_

#include <vector>
#include <cstdint>
#include <string>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

class ZhurinIEdgeSobelSEQ {
 public:
  explicit ZhurinIEdgeSobelSEQ(const InType& in);
  
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
  
  // Основная функция обработки
  std::vector<uint8_t> ApplySobel(const std::vector<uint8_t>& input);
  
  InType input_;
  OutType output_;
  std::string task_type_;
};

}  // namespace zhurin_i_edge_sobel

#endif  // ZHURIN_I_EDGE_SOBEL_SEQ_INCLUDE_OPS_SEQ_HPP_