#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cmath>
#include <vector>

namespace zhurin_i_edge_sobel {

// Ядра Собеля (оптимизированные для вычисления |Gx| + |Gy|)
const int SOBEL_X[3][3] = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1}
};

const int SOBEL_Y[3][3] = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1}
};

ZhurinIEdgeSobelSEQ::ZhurinIEdgeSobelSEQ(const InType& in) : input_(in) {
  SetTypeOfTask(GetStaticTypeOfTask());
}

std::string ZhurinIEdgeSobelSEQ::GetStaticTypeOfTask() {
  return "ZhurinIEdgeSobelSEQ";
}

std::string ZhurinIEdgeSobelSEQ::GetTypeOfTask() const {
  return task_type_;
}

void ZhurinIEdgeSobelSEQ::SetTypeOfTask(const std::string& type) {
  task_type_ = type;
}

const InType& ZhurinIEdgeSobelSEQ::GetInput() const {
  return input_;
}

InType& ZhurinIEdgeSobelSEQ::GetInput() {
  return input_;
}

const OutType& ZhurinIEdgeSobelSEQ::GetOutput() const {
  return output_;
}

OutType& ZhurinIEdgeSobelSEQ::GetOutput() {
  return output_;
}

bool ZhurinIEdgeSobelSEQ::Validation() {
  const auto& in = GetInput();
  
  // Проверяем минимальные размеры для оператора Собеля
  if (in.width < 3 || in.height < 3) {
    return false;
  }
  
  // Проверяем размер массива пикселей
  if (in.pixels.size() != static_cast<size_t>(in.width * in.height)) {
    return false;
  }
  
  return true;
}

bool ZhurinIEdgeSobelSEQ::PreProcessing() {
  output_.clear();
  return true;
}

bool ZhurinIEdgeSobelSEQ::PostProcessing() {
  return true;
}

std::vector<uint8_t> ZhurinIEdgeSobelSEQ::ApplySobel(const std::vector<uint8_t>& input) {
  const int width = input_.width;
  const int height = input_.height;
  
  std::vector<uint8_t> output(width * height, 0);
  
  // Оптимизированный алгоритм: вычисляем градиент как |Gx| + |Gy|
  // и нормализуем делением на 4 (максимальное значение 1020 -> 255)
  
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      // Вычисляем Gx и Gy без использования вложенных циклов
      // для небольшого ускорения
      
      int idx = y * width + x;
      
      int gx = 
          -input[idx - width - 1] - 2 * input[idx - width] - input[idx - width + 1] +
          input[idx + width - 1] + 2 * input[idx + width] + input[idx + width + 1];
      
      int gy = 
          -input[idx - width - 1] - 2 * input[idx - 1] - input[idx + width - 1] +
          input[idx - width + 1] + 2 * input[idx + 1] + input[idx + width + 1];
      
      // Вычисляем величину градиента: |Gx| + |Gy|
      int magnitude = std::abs(gx) + std::abs(gy);
      
      // Нормализация: максимальное значение 1020 (255*4), делим на 4
      magnitude = std::min(255, magnitude / 4);
      
      output[idx] = static_cast<uint8_t>(magnitude);
    }
  }
  
  return output;
}

bool ZhurinIEdgeSobelSEQ::Run() {
  if (!Validation()) {
    return false;
  }
  
  GetOutput() = ApplySobel(GetInput().pixels);
  
  return true;
}

}  // namespace zhurin_i_edge_sobel