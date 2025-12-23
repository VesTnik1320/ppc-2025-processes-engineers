#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"

#include <mpi.h>
#include <algorithm>
#include <cmath>
#include <vector>

namespace zhurin_i_edge_sobel {

// Ядра Собеля
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

ZhurinIEdgeSobelMPI::ZhurinIEdgeSobelMPI(const InType& in) : input_(in) {
  MPI_Comm_rank(MPI_COMM_WORLD, &world_rank_);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size_);
  SetTypeOfTask(GetStaticTypeOfTask());
}

std::string ZhurinIEdgeSobelMPI::GetStaticTypeOfTask() {
  return "ZhurinIEdgeSobelMPI";
}

std::string ZhurinIEdgeSobelMPI::GetTypeOfTask() const {
  return task_type_;
}

void ZhurinIEdgeSobelMPI::SetTypeOfTask(const std::string& type) {
  task_type_ = type;
}

const InType& ZhurinIEdgeSobelMPI::GetInput() const {
  return input_;
}

InType& ZhurinIEdgeSobelMPI::GetInput() {
  return input_;
}

const OutType& ZhurinIEdgeSobelMPI::GetOutput() const {
  return output_;
}

OutType& ZhurinIEdgeSobelMPI::GetOutput() {
  return output_;
}

bool ZhurinIEdgeSobelMPI::Validation() {
  if (world_rank_ == 0) {
    const auto& in = GetInput();
    
    if (in.width < 3 || in.height < 3) {
      return false;
    }
    
    if (in.pixels.size() != static_cast<size_t>(in.width * in.height)) {
      return false;
    }
  }
  
  // Рассылаем результат валидации от процесса 0 всем
  int valid = 1;
  if (world_rank_ == 0) {
    valid = (GetInput().width >= 3 && GetInput().height >= 3 && 
             GetInput().pixels.size() == static_cast<size_t>(GetInput().width * GetInput().height)) ? 1 : 0;
  }
  
  MPI_Bcast(&valid, 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  return valid == 1;
}

bool ZhurinIEdgeSobelMPI::PreProcessing() {
  output_.clear();
  return true;
}

bool ZhurinIEdgeSobelMPI::PostProcessing() {
  return true;
}

void ZhurinIEdgeSobelMPI::BroadcastParameters() {
  int params[2];
  
  if (world_rank_ == 0) {
    params[0] = input_.width;
    params[1] = input_.height;
  }
  
  MPI_Bcast(params, 2, MPI_INT, 0, MPI_COMM_WORLD);
  
  if (world_rank_ != 0) {
    input_.width = params[0];
    input_.height = params[1];
    input_.pixels.resize(input_.width * input_.height);
  }
  
  // Рассылаем пиксели от процесса 0 всем процессам
  MPI_Bcast(input_.pixels.data(), input_.width * input_.height, 
            MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
}

void ZhurinIEdgeSobelMPI::ExchangeBoundaries(std::vector<uint8_t>& local_data) {
  const int width = local_width_;
  
  // Обмен верхней границей с предыдущим процессом
  if (world_rank_ > 0) {
    MPI_Sendrecv(&local_data[width], width, MPI_UNSIGNED_CHAR, 
                 world_rank_ - 1, 0,
                 &local_data[0], width, MPI_UNSIGNED_CHAR, 
                 world_rank_ - 1, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  
  // Обмен нижней границей со следующим процессом
  if (world_rank_ < world_size_ - 1) {
    int last_row = (rows_with_boundaries_ - 1) * width;
    int second_last_row = (rows_with_boundaries_ - 2) * width;
    
    MPI_Sendrecv(&local_data[second_last_row], width, MPI_UNSIGNED_CHAR,
                 world_rank_ + 1, 1,
                 &local_data[last_row], width, MPI_UNSIGNED_CHAR,
                 world_rank_ + 1, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

std::vector<uint8_t> ZhurinIEdgeSobelMPI::ProcessLocalData(const std::vector<uint8_t>& local_data) {
  const int width = local_width_;
  const int height = local_height_;
  
  // Копируем данные для обмена границами
  std::vector<uint8_t> data_with_boundaries = local_data;
  
  // Обмениваемся граничными строками с соседними процессами
  ExchangeBoundaries(data_with_boundaries);
  
  // Результат (без граничных строк)
  std::vector<uint8_t> local_result(rows_per_process_ * width, 0);
  
  // Определяем, с какой строки начинать обработку (учитывая граничные)
  int start_y = (world_rank_ > 0) ? 1 : 0;
  int end_y = (world_rank_ < world_size_ - 1) ? height - 1 : height;
  
  // Обрабатываем строки, исключая граничные
  for (int y = start_y; y < end_y; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      int idx = y * width + x;
      
      // Оптимизированное вычисление градиента
      int gx = 
          -data_with_boundaries[idx - width - 1] - 2 * data_with_boundaries[idx - width] - data_with_boundaries[idx - width + 1] +
          data_with_boundaries[idx + width - 1] + 2 * data_with_boundaries[idx + width] + data_with_boundaries[idx + width + 1];
      
      int gy = 
          -data_with_boundaries[idx - width - 1] - 2 * data_with_boundaries[idx - 1] - data_with_boundaries[idx + width - 1] +
          data_with_boundaries[idx - width + 1] + 2 * data_with_boundaries[idx + 1] + data_with_boundaries[idx + width + 1];
      
      // Вычисляем величину градиента: |Gx| + |Gy|
      int magnitude = std::abs(gx) + std::abs(gy);
      
      // Нормализация
      magnitude = std::min(255, magnitude / 4);
      
      // Сохраняем результат (без учета граничных строк)
      int result_y = y - start_y;
      local_result[result_y * width + x] = static_cast<uint8_t>(magnitude);
    }
  }
  
  return local_result;
}

void ZhurinIEdgeSobelMPI::GatherResults(const std::vector<uint8_t>& local_result) {
  const int width = input_.width;
  const int height = input_.height;
  
  if (world_rank_ == 0) {
    output_.resize(width * height);
  }
  
  // Определяем смещения для каждого процесса
  std::vector<int> recv_counts(world_size_, 0);
  std::vector<int> displacements(world_size_, 0);
  
  // Каждый процесс вычисляет количество строк
  int local_rows = rows_per_process_;
  
  // Собираем информацию о количестве строк от каждого процесса
  MPI_Gather(&local_rows, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
  
  // Вычисляем смещения в выходном массиве
  if (world_rank_ == 0) {
    int offset = 0;
    for (int i = 0; i < world_size_; ++i) {
      displacements[i] = offset * width;
      offset += recv_counts[i];
      recv_counts[i] *= width;  // переводим строки в пиксели
    }
  }
  
  // Собираем все данные в процесс 0
  MPI_Gatherv(local_result.data(), local_rows * width, MPI_UNSIGNED_CHAR,
              output_.data(), recv_counts.data(), displacements.data(),
              MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
}

void ZhurinIEdgeSobelMPI::DistributeData() {
  const int width = input_.width;
  const int height = input_.height;
  
  // Распределение строк по процессам
  rows_per_process_ = height / world_size_;
  int remainder = height % world_size_;
  
  // Определяем стартовую строку для каждого процесса
  if (world_rank_ < remainder) {
    rows_per_process_++;
    local_start_row_ = world_rank_ * rows_per_process_;
  } else {
    local_start_row_ = remainder * (rows_per_process_ + 1) + 
                      (world_rank_ - remainder) * rows_per_process_;
  }
  
  // Добавляем граничные строки для свертки
  rows_with_boundaries_ = rows_per_process_;
  if (world_rank_ > 0) rows_with_boundaries_++;          // верхняя граница
  if (world_rank_ < world_size_ - 1) rows_with_boundaries_++;  // нижняя граница
  
  local_width_ = width;
  local_height_ = rows_with_boundaries_;
  
  // Выделяем память для локальных данных
  std::vector<uint8_t> local_data(local_height_ * local_width_);
  
  // Копируем соответствующие строки изображения
  for (int y = 0; y < local_height_; ++y) {
    int global_y = local_start_row_ + y - (world_rank_ > 0 ? 1 : 0);
    
    // Проверяем границы изображения
    if (global_y >= 0 && global_y < height) {
      for (int x = 0; x < width; ++x) {
        local_data[y * width + x] = input_.pixels[global_y * width + x];
      }
    }
  }
  
  // Обрабатываем локальные данные
  std::vector<uint8_t> local_result = ProcessLocalData(local_data);
  
  // Собираем результаты
  GatherResults(local_result);
}

bool ZhurinIEdgeSobelMPI::Run() {
  if (!Validation()) {
    return false;
  }
  
  // Рассылаем параметры изображения
  BroadcastParameters();
  
  // Распределяем и обрабатываем данные
  DistributeData();
  
  return true;
}

}  // namespace zhurin_i_edge_sobel