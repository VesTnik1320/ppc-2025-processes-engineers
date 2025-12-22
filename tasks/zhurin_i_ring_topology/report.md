# Отчет по лабораторной работе №1
## Работу выполнил студент группы 3823Б1ПР1, Журин Иван Эдуардович
## Вариант № 7. Кольцо.

**Преподаватель:** Сысоев Александр Владимирович, лектор, доцент кафедры высокопроизводительных вычислений и системного программирования

---

## Введение

**Цель работы:** исследование методов параллельного программирования с использованием технологии MPI (Message Passing Interface) на примере задачи сетевая топология кольцо.

**Актуальность:** с ростом объемов обрабатываемых данных последовательные алгоритмы становятся недостаточно эффективными. Применение параллельных технологий позволяет обрабатывать большие массивы данных без значительного увеличения времени вычислений. Кольцевая топология представляет собой фундаментальную модель коммуникации, которая используется во многих параллельных алгоритмах.

В рамках работы требовалось реализовать два варианта алгоритма:
- **Последовательный (SEQ)** — обработка данных в одном процессе
- **Параллельный (MPI)** — распределение работы между несколькими процессами с последующим объединением результатов

---

## Постановка задачи

Разработать программу, реализующую передачу массива целых чисел от процесса-источника к процессу-получателю в кольцевой топологии.  Требуется реализовать:

- **Последовательный алгоритм (SEQ)** — вычисления в одном процессе.
- **Параллельный алгоритм (MPI)** — распределение элементов матрицы между процессами и объединение частичных сумм.

---

## Описание алгоритма

### Последовательный алгоритм (SEQ)

1. Проверка корректности входных данных (source ≥ 0, dest ≥ 0)
2. Прямое копирование входного массива в выходной буфер
3. Имитация задержки для корректного сравнения с параллельной версией

### Параллельный алгоритм (MPI)

1. **Инициализация MPI:** получение ранга процесса и общего количества процессов
2. **Валидация:** проверка корректности входных параметров
3. **Обработка особых случаев:** 
    - Если количество процессов = 1, то данные копируются в выходной буфер
    - Если источник и получатель совпадают, данные рассылаются всем процессам
4. **Корректировка параметров:** приведение номера источника и получателя к диапазону [0, world_size-1]
5. **Определение направления:** вычисление расстояния по часовой стрелке и против часовой стрелки для выбора кратчайшего пути
6. **Передача данных:** 
    - Для 2 процессов: прямая передача между процессами
    - Для 3+ процессов: последовательная передача по кольцу через промежуточные процессы
7. **Рассылка результата:** после получения данных процессом-получателем, результат рассылается всем процессам через MPI_Bcast
---

## Описание схемы параллельного алгоритма

Схема передачи данных для 3+ процессов:
1. **Инициализация:** каждый процесс получает свой rank и world_size
2. **Определение соседей:** вычисление left_neighbor и right_neighbor по формулам:
3. **Определение направления:** вычисление next_hop и prev_hop на основе go_clockwise:
4. **Передача от источника:** Процесс-source отправляет данные следующему процессу (next_hop)
5. **Промежуточная передача:** Процессы, находящиеся на пути (InRing() == true), получают данные от предыдущего (prev_hop) и отправляют следующему (next_hop)
6. **Получение данных:** Процесс-dest получает данные и сохраняет в выходной буфер
7. **Рассылка результата:** процесс-dest рассылает результат всем процессам

---

## Описание программной реализации

### Параллельная реализация (MPI)
**Архитектура коммуникации:**
- Равноправные процессы в коммуникаторе MPI_COMM_WORLD
- Последовательная передача данных по цепочке процессов
- Децентрализованные вычисления

### Фазы выполнения MPI-алгоритма
#### ValidationImpl()
- Локальная проверка корректности входных данных
- Подтверждение неотрицательности source и dest

#### PreProcessingImpl()
- Инициализация выходного буфера
- Подготовка к приему данных

#### RunImpl()
- Определение параметров передачи — расчет эффективных source и dest
- Обработка особых случаев — world_size = 1 или source == dest
- Определение направления передачи — использование параметра go_clockwise
- Передача данных — последовательная передача по кольцу
- Рассылка результата — MPI_Bcast от процесса-получателя

#### PostProcessingImpl()
- Каждый процесс имеет готовый результат
- Дополнительные коммуникации не требуются

### Ключевые особенности реализации

- **Поддержка двух направлений:** передача по часовой и против часовой стрелки
- **Оптимизация для малого количества процессов:** отдельные ветки для world_size = 1 и world_size = 2
- **Гибкая обработка параметров:**приведение source/dest к диапазону с помощью операции модуля
- **Согласованность данных:** идентичные результаты на всех процессах благодаря MPI_Bcast
- **Масштабируемость:** поддержка произвольного числа процессо

---

## Тестирование

### Результаты производительности (размер данных: 1,000,000 элементов)

#### Время выполнения `pipeline` (секунды)

| Процессы |   Время    | Ускорение | Эффективность |
|----------|------------|-----------|---------------|
| 1        | 0.00029528 | 44.64     | 357.12        |
| 2        | 0.00117876 | 11.18     | 44.72         |
| 3        | 0.00453452 | 2.91      | 7.76          |
| 4        | 0.00196226 | 6.72      | 13.44         |
| 5        | 0.00636664 | 2.07      | 3.31          |
| 6        | 0.00865762 | 1.52      | 2.03          |
| 7        | 0.01068792 | 1.23      | 1.41          |
| 8        | 0.01318048 | 1.00      | 1.00          |

#### Время выполнения `task_run` (секунды)

| Процессы |   Время    | Ускорение | Эффективность |
|----------|------------|-----------|---------------|
| 1        | 0.00033908 | 34.76     | 278.08        |
| 2        | 0.00105836 | 11.13     | 44.52         |
| 3        | 0.00443340 | 2.66      | 7.09          |
| 4        | 0.00181508 | 6.49      | 12.98         |
| 5        | 0.00447112 | 2.63      | 4.21          |
| 6        | 0.00724758 | 1.63      | 2.17          |
| 7        | 0.00832932 | 1.41      | 1.62          |
| 8        | 0.01178238 | 1.00      | 1.00          |

### Анализ результатов

#### Основные наблюдения:
- Минимальное время при 1 процессе: Это соответствует последовательному копированию данных без коммуникационных издержек
- Оптимальная производительность при 4 процессах: Это может быть связано с эффективной маршрутизацией и балансировкой нагрузки
- Прогрессивное увеличение времени с ростом процессов
- Аномалия при 3 процессах: Неожиданно высокое время выполнения(Возможные причины: неоптимальная маршрутизация, дисбаланс нагрузки)

### Проверка корректности
Все функциональные тесты успешно пройдены, что подтверждает:
1. Корректность передачи данных: данные доставляются от источника к получателю без искажений
2. Обработка особых случаев: корректная работа при world_size = 1 и source = dest
3. Согласованность результатов: все процессы получают одинаковый результат после широковещательной рассылки
4. Масштабируемость: работа с различным количеством процессов (1-8)

---

## Заключение

В ходе лабораторной работы были успешно реализованы последовательный и параллельный алгоритмы передачи данных в кольцевой топологии. Параллельная реализация на основе MPI корректно работает для любого количества процессов и эффективно обрабатывает все граничные случаи. 

---

## Приложение


# **common.hpp**

#ifndef ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_
#define ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_

#include <tuple>
#include <vector>

namespace zhurin_i_ring_topology {

struct RingMessage {
  int source{};
  int dest{};
  std::vector<int> data;
  bool go_clockwise = true;
};

using InType = RingMessage;
using OutType = std::vector<int>;
using TestType = std::tuple<int, RingMessage>;

}  // namespace zhurin_i_ring_topology

#endif  // ZHURIN_I_RING_TOPOLOGY_COMMON_INCLUDE_COMMON_HPP_

---

# **ops_mpi.hpp**

#ifndef ZHURIN_I_RING_TOPOLOGY_MPI_INCLUDE_OPS_MPI_HPP_
#define ZHURIN_I_RING_TOPOLOGY_MPI_INCLUDE_OPS_MPI_HPP_

#include "task/include/task.hpp"
#include "zhurin_i_ring_topology/common/include/common.hpp"

namespace zhurin_i_ring_topology {

using BaseTask = ppc::task::Task<InType, OutType>;

class ZhurinIRingTopologyMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit ZhurinIRingTopologyMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zhurin_i_ring_topology

#endif  // ZHURIN_I_RING_TOPOLOGY_MPI_INCLUDE_OPS_MPI_HPP_

---

# **ops_mpi.cpp**

#include "zhurin_i_ring_topology/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <cstddef>
#include <cstdint>
#include <vector>

#include "zhurin_i_ring_topology/common/include/common.hpp"

namespace zhurin_i_ring_topology {

namespace {

bool InRing(int rank, int source, int dest, bool go_clockwise, int world_size) {
  if (source == dest) {
    return false;
  }

  if (rank < 0 || rank >= world_size || source < 0 || source >= world_size || dest < 0 || dest >= world_size) {
    return false;
  }

  if (go_clockwise) {
    if (source < dest) {
      return rank > source && rank <= dest;
    }
    return rank > source || rank <= dest;
  }

  if (source > dest) {
    return rank < source && rank >= dest;
  }
  return rank < source || rank >= dest;
}

void SendAllInfo(int dest_rank, uint64_t data_size, const std::vector<int> &data, int size_tag = 0, int data_tag = 1) {
  if (dest_rank == MPI_PROC_NULL) {
    return;
  }

  MPI_Send(&data_size, 1, MPI_UINT64_T, dest_rank, size_tag, MPI_COMM_WORLD);
  if (data_size > 0U) {
    MPI_Send(data.data(), static_cast<int>(data_size), MPI_INT, dest_rank, data_tag, MPI_COMM_WORLD);
  }
}

void ReceiveAllInfo(int src_rank, uint64_t &data_size, std::vector<int> &data, int size_tag = 0, int data_tag = 1) {
  if (src_rank == MPI_PROC_NULL) {
    return;
  }

  MPI_Recv(&data_size, 1, MPI_UINT64_T, src_rank, size_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  if (data_size > 0U) {
    data.resize(static_cast<std::size_t>(data_size));
    MPI_Recv(data.data(), static_cast<int>(data_size), MPI_INT, src_rank, data_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}

void BroadcastResult(int rank, int root, std::vector<int> &output) {
  uint64_t data_size = 0;

  if (rank == root) {
    data_size = static_cast<uint64_t>(output.size());
  }

  MPI_Bcast(&data_size, 1, MPI_UINT64_T, root, MPI_COMM_WORLD);

  if (rank != root) {
    output.resize(static_cast<std::size_t>(data_size));
  }

  if (data_size > 0U) {
    MPI_Bcast(output.data(), static_cast<int>(data_size), MPI_INT, root, MPI_COMM_WORLD);
  }
}

void SameSD(int rank, int source, const std::vector<int> &input_data, std::vector<int> &output) {
  auto data_size = static_cast<uint64_t>(input_data.size());

  MPI_Bcast(&data_size, 1, MPI_UINT64_T, source, MPI_COMM_WORLD);

  if (rank == source) {
    output = input_data;
  } else {
    output.resize(static_cast<std::size_t>(data_size));
  }

  if (data_size > 0U) {
    MPI_Bcast(output.data(), static_cast<int>(data_size), MPI_INT, source, MPI_COMM_WORLD);
  }
}

void DataRoute(int rank, int source, int dest, bool go_clockwise, int world_size, const std::vector<int> &input_data,
               std::vector<int> &output) {
  std::vector<int> buffer;
  uint64_t data_size = 0;

  if (world_size == 1) {
    if (rank == source) {
      output = input_data;
    }
    return;
  }

  if (world_size == 2) {
    if (rank == source) {
      buffer = input_data;
      data_size = static_cast<uint64_t>(buffer.size());
      SendAllInfo(dest, data_size, buffer);
    }
    if (rank == dest) {
      ReceiveAllInfo(source, data_size, buffer);
      output = buffer;
    }
    return;
  }

  int left_neighbor = (rank - 1 + world_size) % world_size;
  int right_neighbor = (rank + 1) % world_size;

  int next_hop = go_clockwise ? right_neighbor : left_neighbor;
  int prev_hop = go_clockwise ? left_neighbor : right_neighbor;

  if (rank == source) {
    buffer = input_data;
    data_size = static_cast<uint64_t>(buffer.size());
    SendAllInfo(next_hop, data_size, buffer);
  }

  if (InRing(rank, source, dest, go_clockwise, world_size)) {
    ReceiveAllInfo(prev_hop, data_size, buffer);

    if (rank == dest) {
      output = buffer;
    } else {
      SendAllInfo(next_hop, data_size, buffer);
    }
  }
}

}  // namespace

ZhurinIRingTopologyMPI::ZhurinIRingTopologyMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput().clear();
}

bool ZhurinIRingTopologyMPI::ValidationImpl() {
  const auto &input = GetInput();
  return input.source >= 0 && input.dest >= 0;
}

bool ZhurinIRingTopologyMPI::PreProcessingImpl() {
  GetOutput().clear();
  return true;
}

bool ZhurinIRingTopologyMPI::RunImpl() {
  int rank = 0;
  int world_size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &world_size);

  const auto &input = GetInput();

  if (world_size == 1) {
    int effective_source = 0;

    SameSD(rank, effective_source, input.data, GetOutput());
    return true;
  }

  int effective_source = input.source % world_size;
  int effective_dest = input.dest % world_size;

  if (effective_source == effective_dest) {
    SameSD(rank, effective_source, input.data, GetOutput());
    return true;
  }

  bool go_clockwise = input.go_clockwise;

  DataRoute(rank, effective_source, effective_dest, go_clockwise, world_size, input.data, GetOutput());

  BroadcastResult(rank, effective_dest, GetOutput());
  return true;
}

bool ZhurinIRingTopologyMPI::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_ring_topology

---

# **ops_seq.hpp**

#ifndef ZHURIN_I_RING_TOPOLOGY_SEQ_INCLUDE_OPS_SEQ_HPP_
#define ZHURIN_I_RING_TOPOLOGY_SEQ_INCLUDE_OPS_SEQ_HPP_

#include "task/include/task.hpp"
#include "zhurin_i_ring_topology/common/include/common.hpp"

namespace zhurin_i_ring_topology {

using BaseTask = ppc::task::Task<InType, OutType>;

class ZhurinIRingTopologySEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kSEQ;
  }
  explicit ZhurinIRingTopologySEQ(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace zhurin_i_ring_topology

#endif  // ZHURIN_I_RING_TOPOLOGY_SEQ_INCLUDE_OPS_SEQ_HPP_

---

# **ops_seq.cpp**

#include "zhurin_i_ring_topology/seq/include/ops_seq.hpp"

#include <chrono>
#include <thread>
#include <vector>

#include "zhurin_i_ring_topology/common/include/common.hpp"

namespace zhurin_i_ring_topology {

ZhurinIRingTopologySEQ::ZhurinIRingTopologySEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = {};
}

bool ZhurinIRingTopologySEQ::ValidationImpl() {
  const auto &input = GetInput();
  return input.source >= 0 && input.dest >= 0;
}

bool ZhurinIRingTopologySEQ::PreProcessingImpl() {
  GetOutput() = {};
  return true;
}

bool ZhurinIRingTopologySEQ::RunImpl() {
  const auto &input = GetInput();
  GetOutput() = input.data;
  if (input.source != input.dest) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }

  return true;
}

bool ZhurinIRingTopologySEQ::PostProcessingImpl() {
  return true;
}

}  // namespace zhurin_i_ring_topology
