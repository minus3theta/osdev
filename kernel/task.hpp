#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

struct TaskContext {
  uint64_t cr3, rip, rflags, reserved1;
  uint64_t cs, ss, fs, gs;
  uint64_t rax, rbx, rcx, rdx, rdi, rsi, rsp, rbp;
  uint64_t r8, r9, r10, r11, r12, r13, r14, r15;
  std::array<int8_t, 512> fxsave_area;
} __attribute__((packed));

alignas(16) inline TaskContext task_b_ctx, task_a_ctx;

void InitializeTask();

using TaskFunc = void(uint64_t, int64_t);

class Task {
public:
  static const size_t kDefaultStackBytes = 4096;
  Task(uint64_t id);
  Task &InitContext(TaskFunc *f, int64_t data);
  TaskContext &Context();

private:
  uint64_t id;
  std::vector<uint64_t> stack;
  alignas(16) TaskContext context;
};

class TaskManager {
public:
  TaskManager();
  Task &NewTask();
  void SwitchTask();

private:
  std::vector<std::unique_ptr<Task>> tasks;
  uint64_t latest_id{0};
  size_t current_task_index{0};
};

inline TaskManager *task_manager;
