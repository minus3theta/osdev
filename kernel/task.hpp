#pragma once

#include <array>
#include <cstdint>
#include <deque>
#include <memory>
#include <optional>
#include <vector>

#include "error.hpp"
#include "fat.hpp"
#include "message.hpp"

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

class TaskManager;

struct FileMapping {
  int fd;
  uint64_t vaddr_begin, vaddr_end;
};

class Task {
public:
  static const int kDefaultLevel = 1;
  static const size_t kDefaultStackBytes = 4096;

  Task(uint64_t id);
  Task &InitContext(TaskFunc *f, int64_t data);
  TaskContext &Context();
  uint64_t &OSStackPointer();
  uint64_t ID() const;
  Task &Sleep();
  Task &Wakeup();

  void SendMessage(const Message &msg);
  std::optional<Message> ReceiveMessage();
  std::vector<std::unique_ptr<::FileDescriptor>> &Files();
  uint64_t DPagingBegin() const;
  void SetDPagingBegin(uint64_t v);
  uint64_t DPagingEnd() const;
  void SetDPagingEnd(uint64_t v);
  uint64_t FileMapEnd() const;
  void SetFileMapEnd(uint64_t v);
  std::vector<FileMapping> &FileMaps();

  int Level() const { return level; }
  bool Running() const { return running; }

private:
  uint64_t id;
  std::vector<uint64_t> stack;
  alignas(16) TaskContext context;
  uint64_t os_stack_ptr;
  std::deque<Message> msgs;
  unsigned int level{kDefaultLevel};
  bool running{false};
  std::vector<std::unique_ptr<::FileDescriptor>> files{};
  uint64_t dpaging_begin{0}, dpaging_end{0};
  uint64_t file_map_end{0};
  std::vector<FileMapping> file_maps{};

  Task &SetLevel(int level) {
    this->level = level;
    return *this;
  }

  Task &SetRunning(bool running) {
    this->running = running;
    return *this;
  }

  friend TaskManager;
};

class TaskManager {
public:
  static const int kMaxLevel = 3;

  TaskManager();
  Task &NewTask();
  void SwitchTask(const TaskContext &ctx_stack);

  void Sleep(Task *task);
  Error Sleep(uint64_t id);
  void Wakeup(Task *task, int level = -1);
  Error Wakeup(uint64_t id, int level = -1);

  Task &CurrentTask() const;
  Error SendMessage(uint64_t id, const Message &msg);

private:
  std::vector<std::unique_ptr<Task>> tasks;
  uint64_t latest_id{0};
  std::array<std::deque<Task *>, kMaxLevel + 1> running{};
  int current_level{kMaxLevel};
  bool level_changed{false};

  void ChangeLevelRunning(Task *task, int level);
  Task *RotateCurrentRunQueue(bool current_sleep);
};

inline TaskManager *task_manager;

__attribute__((no_caller_saved_registers)) extern "C" uint64_t
GetCurrentTaskOSStackPointer();
