# Отчет по лабораторной работе №3
## Работу выполнил студент группы 3823Б1ПР1, Журин Иван Эдуардович
## Вариант № 29. Выделение ребер на изображении с использованием оператора Собеля..

**Преподаватель:** Сысоев Александр Владимирович, лектор, доцент кафедры высокопроизводительных вычислений и системного программирования

---
## Введение

**Цель работы:** разработка и исследование последовательного и параллельного алгоритмов выделения границ на изображении с использованием оператора Собеля и технологии MPI (Message Passing Interface).

**Актуальность:** задачи обработки изображений широко применяются в компьютерном зрении, анализе данных и системах распознавания образов. Алгоритм Собеля является классическим методом выделения границ, однако при обработке изображений большого разрешения вычислительная нагрузка существенно возрастает. Использование параллельных технологий позволяет ускорить вычисления за счёт распределения данных между процессами.

В рамках работы требовалось реализовать два варианта алгоритма:

- **Последовательный (SEQ)** — обработка изображения в одном процессе.
- **Параллельный (MPI)** — разбиение изображения по строкам между процессами с использованием halo-областей.

---

## Постановка задачи

Разработать программу, реализующую выделение границ на полутоновом изображении с использованием оператора Собеля. Требуется:

- **Последовательный алгоритм (SEQ)** — вычисление градиентов для всего изображения.
- **Параллельный алгоритм (MPI)** — распределение строк изображения между процессами с последующим объединением результата.

**Входные данные:**
- Одномерный массив пикселей изображения
- Высота изображения
- Ширина изображения
- Пороговое значение градиента

**Выходные данные:**
- Массив пикселей, содержащий значения модуля градиента, превышающие порог

---

## Описание алгоритма

### Последовательный алгоритм (SEQ)

1. Инициализация выходного массива размером `height × width`.
2. Для каждого пикселя изображения вычисляется:
   - горизонтальный градиент `Gx` с использованием ядра Собеля `SobelX`.
   - вертикальный градиент `Gy` с использованием ядра Собеля `SobelY`.
3. Вычисление модуля градиента: magnitude = sqrt(Gx² + Gy²)
4. Применение пороговой фильтрации:
- если `magnitude > threshold`, результат сохраняется.
- иначе записывается `0`.
5. Для граничных пикселей выполняется проверка выхода за пределы изображения.

### Параллельный алгоритм (MPI)

1. **Инициализация MPI:** получение `rank` и `world_size`.
2. **Рассылка параметров:** высота, ширина изображения и порог передаются всем процессам с помощью `MPI_Bcast`.
3. **Распределение строк изображения:**
- изображение делится по строкам между процессами.
- остаточные строки распределяются между первыми процессами.
4. **Halo-области:**
- каждый процесс получает дополнительные строки сверху и снизу.
- halo используется для корректного вычисления градиентов на границах блоков.
5. **Передача данных:** используется `MPI_Scatterv` для передачи строк изображения с учётом halo.
6. **Локальные вычисления:** каждый процесс вычисляет градиенты только для своих строк.
7. **Сбор результатов:** локальные результаты объединяются в итоговый массив с помощью `MPI_Allgatherv`.

---

## Ключевые особенности реализации

- **Использование оператора Собеля:** ядра `3×3` для вычисления градиентов.
- **Пороговая фильтрация:** подавление слабых границ.
- **Halo-обмен:** корректная обработка граничных строк.
- **MPI_Scatterv / MPI_Allgatherv:** гибкое распределение и сбор данных.
- **Согласованность результатов:** совпадение SEQ и MPI версий.
- **Масштабируемость:** поддержка произвольного числа процессов.

---

## Тестирование

### Функциональное тестирование

Тестирование проводилось на наборе из 10 изображений. Подтверждено:

1. корректность вычисления градиентов
2. совпадение результатов SEQ и MPI
3. корректная обработка граничных пикселей
4. отсутствие искажений при сборе результата
5. работа с изображениями различного размера

### Тестирование производительности

Результаты тестов MPI на изображении 2000×2000:

#### Время выполнения `pipeline` (секунды)

| Процессы | Время | Ускорение | Эффективность |
|-----------|-------|-----------|---------------|
| 1         | 0.0846| 1.000     | 1.000         |
| 2         | 0.0496| 1.706     | 0.853         |
| 3         | 0.0384| 2.203     | 0.734         |
| 4         | 0.0322| 2.627     | 0.657         |
| 5         | 0.0347| 2.438     | 0.488         |
| 6         | 0.0357| 2.371     | 0.395         |
| 7         | 0.0310| 2.730     | 0.390         |
| 8         | 0.0331| 2.556     | 0.319         |

#### Время выполнения `task_run` (секунды)

| Процессы | Время | Ускорение | Эффективность |
|-----------|-------|-----------|---------------|
| 1         | 0.0866| 1.000     | 1.000         |
| 2         | 0.0490| 1.767     | 0.883         |
| 3         | 0.0373| 2.321     | 0.774         |
| 4         | 0.0331| 2.616     | 0.654         |
| 5         | 0.0367| 2.361     | 0.472         |
| 6         | 0.0325| 2.664     | 0.444         |
| 7         | 0.0310| 2.794     | 0.399         |
| 8         | 0.0335| 2.585     | 0.323         |

### Анализ результатов

1. При увеличении числа процессов ускорение растет не линейно.
2. Эффективность падает из-за накладных расходов на передачу halo-областей и синхронизацию.
3. Оптимальное количество процессов для данного размера изображения — 3–4.
4. Последовательный алгоритм работает корректно и может служить эталоном для проверки MPI-версии.

---

## Заключение

В ходе лабораторной работы были успешно реализованы:

- последовательный алгоритм выделения границ на изображении,
- параллельный MPI-алгоритм с разбиением по строкам и использованием halo-областей.

Параллельная реализация корректно обрабатывает любые размеры изображений и гарантирует идентичные результаты на всех процессах. Рост числа процессов ускоряет вычисления, но эффективность снижается из-за коммуникационных накладных расходов.

---

## Приложение


# **common.hpp**
#pragma once

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <tuple>
#include <vector>

#include "task/include/task.hpp"

namespace zhurin_i_edge_sobel {

enum class ImageSet : std::uint8_t { kTest1, kTest2, kTest3, kTest4, kTest5, kTest6, kTest7, kTest8, kTest9, kTest10 };

using InType = std::tuple<std::vector<int>, int, int, int>;
using OutType = std::vector<int>;
using TestType = std::tuple<ImageSet, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

inline std::tuple<std::vector<int>, int, int> ReadImageFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    return std::make_tuple(std::vector<int>(), 0, 0);
  }

  int height = 0;
  int width = 0;

  file >> height;
  file >> width;

  std::vector<int> pixels;
  pixels.reserve(static_cast<size_t>(height) * width);

  for (int i = 0; i < height; ++i) {
    for (int j = 0; j < width; ++j) {
      int pixel = 0;
      if (!(file >> pixel)) {
        return std::make_tuple(std::vector<int>(), 0, 0);
      }
      pixels.push_back(pixel);
    }
  }

  file.close();
  return std::make_tuple(pixels, height, width);
}

inline std::string GetDirectoryPath(const std::string &full_path) {
  std::string result = full_path;
  const std::string json_part = "settings.json";

  if (result.size() >= json_part.size()) {
    size_t pos = result.size() - json_part.size();
    if (result.compare(pos, json_part.size(), json_part) == 0) {
      result.erase(pos, json_part.size());
    }
  }

  return result;
}

inline std::string GetTestFilename(ImageSet type, const std::string &folder) {
  int test_num = 0;
  switch (type) {
    case ImageSet::kTest1:
      test_num = 1;
      break;
    case ImageSet::kTest2:
      test_num = 2;
      break;
    case ImageSet::kTest3:
      test_num = 3;
      break;
    case ImageSet::kTest4:
      test_num = 4;
      break;
    case ImageSet::kTest5:
      test_num = 5;
      break;
    case ImageSet::kTest6:
      test_num = 6;
      break;
    case ImageSet::kTest7:
      test_num = 7;
      break;
    case ImageSet::kTest8:
      test_num = 8;
      break;
    case ImageSet::kTest9:
      test_num = 9;
      break;
    case ImageSet::kTest10:
      test_num = 10;
      break;
    default:
      test_num = 1;
  }

  return folder + "/test" + std::to_string(test_num) + ".txt";
}

inline std::string GetTestFilename(ImageSet type) {
  int test_num = static_cast<int>(type) + 1;  // kTest1 -> 1, kTest2 -> 2 и т.д.
  return "test" + std::to_string(test_num) + ".txt";
}

inline std::tuple<std::vector<int>, int, int, int> GenerateTestData(ImageSet type) {
  std::string full_path = GetDirectoryPath(PPC_SETTINGS_zhurin_i_edge_sobel) + "tasks/zhurin_i_edge_sobel/data/cases/" +
                          GetTestFilename(type);

  std::tuple<std::vector<int>, int, int> read_result = ReadImageFile(full_path);

  std::vector<int> pixels = std::get<0>(read_result);
  int height = std::get<1>(read_result);
  int width = std::get<2>(read_result);

  int threshold = 100;

  return std::make_tuple(pixels, height, width, threshold);
}

inline std::vector<int> GenerateExpectedOutput(ImageSet type) {
  std::string full_path = GetDirectoryPath(PPC_SETTINGS_zhurin_i_edge_sobel) +
                          "tasks/zhurin_i_edge_sobel/data/expected/" + GetTestFilename(type);

  std::tuple<std::vector<int>, int, int> read_result = ReadImageFile(full_path);

  return std::get<0>(read_result);
}

}  // namespace zhurin_i_edge_sobel

---

#**ops_mpi.hpp:**

#pragma once

// #include <mpi.h>

#include <cmath>
// #include <cstddef>
// #include <tuple>
#include <vector>

#include "task/include/task.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

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
  [[nodiscard]] int GradientX(int x, int y) const;
  [[nodiscard]] int GradientY(int x, int y) const;
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

---

#**ops_mpi.hpp:**

#include "zhurin_i_edge_sobel/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <vector>

#include "zhurin_i_edge_sobel/common/include/common.hpp"
namespace zhurin_i_edge_sobel {

ZhurinIEdgeSobelMPI::ZhurinIEdgeSobelMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());

  int rank = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    GetInput() = in;
    input_pixels_ = std::get<0>(in);
    height_ = std::get<1>(in);
    width_ = std::get<2>(in);
    threshold_ = std::get<3>(in);
  }

  GetOutput().clear();
}

bool ZhurinIEdgeSobelMPI::ValidationImpl() {
  return true;
}
bool ZhurinIEdgeSobelMPI::PreProcessingImpl() {
  return true;
}
bool ZhurinIEdgeSobelMPI::PostProcessingImpl() {
  return true;
}

bool ZhurinIEdgeSobelMPI::RunImpl() {
  BroadcastParameters();
  DistributeRows();
  auto local_result = LocalGradientsComputing();
  GatherResults(local_result);
  return true;
}

void ZhurinIEdgeSobelMPI::BroadcastParameters() {
  std::array<int, 3> params{height_, width_, threshold_};
  MPI_Bcast(params.data(), static_cast<int>(params.size()), MPI_INT, 0, MPI_COMM_WORLD);

  height_ = params[0];
  width_ = params[1];
  threshold_ = params[2];
}

void ZhurinIEdgeSobelMPI::RowDistributionComputing(int world_rank, int world_size, int &base_rows, int &remainder,
                                                   int &real_rows, int &need_top_halo, int &need_bottom_halo,
                                                   int &total_rows) {
  base_rows = height_ / world_size;
  remainder = height_ % world_size;

  real_rows = base_rows + (world_rank < remainder ? 1 : 0);
  local_height_ = real_rows;

  need_top_halo = (world_rank > 0) ? 1 : 0;
  need_bottom_halo = (world_rank < world_size - 1) ? 1 : 0;

  total_rows = real_rows + need_top_halo + need_bottom_halo;
  local_height_with_halo_ = total_rows;

  local_pixels_.assign(static_cast<size_t>(total_rows) * width_, 0);
}

void ZhurinIEdgeSobelMPI::SendParameters(int world_rank, int world_size, int base_rows, int remainder,
                                         std::vector<int> &real_rows_per_proc, std::vector<int> &send_counts,
                                         std::vector<int> &send_displs) const {
  if (world_rank != 0) {
    return;
  }

  int current_row = 0;
  for (int proc_idx = 0; proc_idx < world_size; ++proc_idx) {
    int rows = base_rows + (proc_idx < remainder ? 1 : 0);
    real_rows_per_proc[proc_idx] = rows;

    int top = (proc_idx > 0) ? 1 : 0;
    int bottom = (proc_idx < world_size - 1) ? 1 : 0;

    int start = current_row - top;
    int end = std::min(current_row + rows + bottom - 1, height_ - 1);

    int count_rows = end - start + 1;
    send_counts[proc_idx] = count_rows * width_;
    send_displs[proc_idx] = start * width_;

    current_row += rows;
  }
}

void ZhurinIEdgeSobelMPI::DistributeRows() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int base = 0;
  int rem = 0;
  int real = 0;
  int top = 0;
  int bottom = 0;
  int total = 0;

  RowDistributionComputing(rank, size, base, rem, real, top, bottom, total);

  std::vector<int> send_counts(static_cast<size_t>(size), 0);
  std::vector<int> send_displs(static_cast<size_t>(size), 0);
  std::vector<int> real_rows_per_proc(static_cast<size_t>(size), 0);

  SendParameters(rank, size, base, rem, real_rows_per_proc, send_counts, send_displs);

  MPI_Scatterv(rank == 0 ? input_pixels_.data() : nullptr, send_counts.data(), send_displs.data(), MPI_INT,
               local_pixels_.data(), total * width_, MPI_INT, 0, MPI_COMM_WORLD);
}

std::vector<int> ZhurinIEdgeSobelMPI::LocalGradientsComputing() {
  std::vector<int> result(static_cast<size_t>(local_height_) * width_, 0);
  int offset = (local_height_with_halo_ > local_height_) ? 1 : 0;

  for (int iy = 0; iy < local_height_; ++iy) {
    for (int ix = 0; ix < width_; ++ix) {
      int gx = GradientX(ix, iy + offset);
      int gy = GradientY(ix, iy + offset);
      int mag = static_cast<int>(std::sqrt((gx * gx) + (gy * gy)));
      result[(iy * width_) + ix] = (mag > threshold_) ? mag : 0;
    }
  }
  return result;
}

int ZhurinIEdgeSobelMPI::GradientX(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < local_height_with_halo_) {
        sum += local_pixels_[(ny * width_) + nx] * kSobelX[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

int ZhurinIEdgeSobelMPI::GradientY(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < local_height_with_halo_) {
        sum += local_pixels_[(ny * width_) + nx] * kSobelY[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

void ZhurinIEdgeSobelMPI::GatherResults(const std::vector<int> &local_result) {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int local_size = static_cast<int>(local_result.size());
  std::vector<int> recv_counts(size);
  MPI_Allgather(&local_size, 1, MPI_INT, recv_counts.data(), 1, MPI_INT, MPI_COMM_WORLD);

  std::vector<int> displs(size);
  int total_size = 0;
  for (int i = 0; i < size; ++i) {
    displs[i] = total_size;
    total_size += recv_counts[i];
  }

  GetOutput().resize(static_cast<size_t>(total_size));
  MPI_Allgatherv(local_result.data(), local_size, MPI_INT, GetOutput().data(), recv_counts.data(), displs.data(),
                 MPI_INT, MPI_COMM_WORLD);
}

void ZhurinIEdgeSobelMPI::LocalRowsComputing(int /*unused*/, int /*unused*/) {}
void ZhurinIEdgeSobelMPI::DataDistribution(int /*unused*/, const std::vector<int> & /*unused1*/,
                                           const std::vector<int> & /*unused2*/) {}

}  // namespace zhurin_i_edge_sobel

---

#**ops_seq.hpp:**

#pragma once

#include <vector>

#include "task/include/task.hpp"
#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

class ZhurinIEdgeSobelSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }

  explicit ZhurinIEdgeSobelSEQ(const InType &in);

 private:
  bool ValidationImpl() override {
    return true;
  }
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override {
    return true;
  }

  [[nodiscard]] int GradientX(int x, int y) const;
  [[nodiscard]] int GradientY(int x, int y) const;

  int height_ = 0;
  int width_ = 0;
  int threshold_ = 0;
  std::vector<int> input_pixels_;
};

}  // namespace zhurin_i_edge_sobel

---

#**ops_mpi.cpp:**

#include "zhurin_i_edge_sobel/seq/include/ops_seq.hpp"

#include <cmath>
#include <cstddef>
#include <vector>

#include "zhurin_i_edge_sobel/common/include/common.hpp"

namespace zhurin_i_edge_sobel {

const std::vector<std::vector<int>> kSobelX = {{-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1}};
const std::vector<std::vector<int>> kSobelY = {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}};

ZhurinIEdgeSobelSEQ::ZhurinIEdgeSobelSEQ(const InType &in)
    : height_(std::get<1>(in)), width_(std::get<2>(in)), threshold_(std::get<3>(in)), input_pixels_(std::get<0>(in)) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool ZhurinIEdgeSobelSEQ::PreProcessingImpl() {
  GetOutput().assign(static_cast<size_t>(height_) * width_, 0);
  return true;
}

bool ZhurinIEdgeSobelSEQ::RunImpl() {
  auto &output = GetOutput();

  for (int iy = 0; iy < height_; ++iy) {
    for (int ix = 0; ix < width_; ++ix) {
      int gx = GradientX(ix, iy);
      int gy = GradientY(ix, iy);
      int mag = static_cast<int>(std::sqrt((gx * gx) + (gy * gy)));
      output[(iy * width_) + ix] = (mag > threshold_) ? mag : 0;
    }
  }
  return true;
}

int ZhurinIEdgeSobelSEQ::GradientX(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
        sum += input_pixels_[(ny * width_) + nx] * kSobelX[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

int ZhurinIEdgeSobelSEQ::GradientY(int x, int y) const {
  int sum = 0;
  for (int ky = -1; ky <= 1; ++ky) {
    for (int kx = -1; kx <= 1; ++kx) {
      int nx = x + kx;
      int ny = y + ky;
      if (nx >= 0 && nx < width_ && ny >= 0 && ny < height_) {
        sum += input_pixels_[(ny * width_) + nx] * kSobelY[ky + 1][kx + 1];
      }
    }
  }
  return sum;
}

}  // namespace zhurin_i_edge_sobel

---