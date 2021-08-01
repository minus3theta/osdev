#include "task.hpp"

#include <algorithm>
#include <cstddef>

#include "asmfunc.hpp"
#include "error.hpp"
#include "segment.hpp"
#include "timer.hpp"

void InitializeTask() {
  task_manager = new TaskManager;

  __asm__("cli");
  timer_manager->AddTimer(Timer{timer_manager->CurrentTick() + kTaskTimerPeriod,
                                kTaskTimerValue, 1});
  __asm__("sti");
}

namespace {
template <class T, class U> void Erase(T &c, const U &value) {
  auto it = std::remove(c.begin(), c.end(), value);
  c.erase(it, c.end());
}

void TaskIdle(uint64_t task_id, int64_t data) {
  while (true) {
    __asm__("hlt");
  }
}
} // namespace

Task::Task(uint64_t id) : id(id) {}

Task &Task::InitContext(TaskFunc *f, int64_t data) {
  const size_t stack_size = kDefaultStackBytes / sizeof(stack[0]);
  stack.resize(stack_size);
  uint64_t stack_end = reinterpret_cast<uint64_t>(&stack[stack_size]);

  memset(&context, 0, sizeof(context));
  context.cr3 = GetCR3();
  context.rflags = 0x202;
  context.cs = kKernelCS;
  context.ss = kKernelSS;
  context.rsp = (stack_end & ~0xflu) - 8;
  context.rip = reinterpret_cast<uint64_t>(f);
  context.rdi = id;
  context.rsi = data;

  *reinterpret_cast<uint32_t *>(&context.fxsave_area[24]) = 0x1f80;

  return *this;
}

TaskContext &Task::Context() {
  return context;
}

uint64_t Task::ID() const {
  return id;
}

Task &Task::Sleep() {
  task_manager->Sleep(this);
  return *this;
}

Task &Task::Wakeup() {
  task_manager->Wakeup(this);
  return *this;
}

std::vector<std::unique_ptr<::FileDescriptor>> &Task::Files() {
  return files;
}

void Task::SendMessage(const Message &msg) {
  msgs.push_back(msg);
  Wakeup();
}

std::optional<Message> Task::ReceiveMessage() {
  if (msgs.empty()) {
    return std::nullopt;
  }

  auto m = msgs.front();
  msgs.pop_front();
  return m;
}

TaskManager::TaskManager() {
  Task &task = NewTask().SetLevel(current_level).SetRunning(true);
  running[current_level].push_back(&task);

  Task &idle = NewTask().InitContext(TaskIdle, 0).SetLevel(0).SetRunning(true);
  running[0].push_back(&idle);
}

Task &TaskManager::NewTask() {
  ++latest_id;
  return *tasks.emplace_back(new Task{latest_id});
}

void TaskManager::SwitchTask(const TaskContext &current_ctx) {
  TaskContext &task_ctx = task_manager->CurrentTask().Context();
  memcpy(&task_ctx, &current_ctx, sizeof(TaskContext));
  Task *current_task = RotateCurrentRunQueue(false);
  if (&CurrentTask() != current_task) {
    RestoreContext(&CurrentTask().Context());
  }
}

void TaskManager::Sleep(Task *task) {
  if (!task->Running()) {
    return;
  }

  task->SetRunning(false);

  if (task == running[current_level].front()) {
    Task *current_task = RotateCurrentRunQueue(true);
    SwitchContext(&CurrentTask().Context(), &current_task->Context());
    return;
  }

  Erase(running[task->Level()], task);
}

Error TaskManager::Sleep(uint64_t id) {
  auto it = std::find_if(tasks.begin(), tasks.end(),
                         [id](const auto &t) { return t->ID() == id; });
  if (it == tasks.end()) {
    return MAKE_ERROR(Error::kNoSuchTask);
  }

  Sleep(it->get());
  return MAKE_ERROR(Error::kSuccess);
}

void TaskManager::Wakeup(Task *task, int level) {
  if (task->Running()) {
    ChangeLevelRunning(task, level);
    return;
  }

  if (level < 0) {
    level = task->Level();
  }

  task->SetLevel(level);
  task->SetRunning(true);

  running[level].push_back(task);
  if (level > current_level) {
    level_changed = true;
  }
}

Error TaskManager::Wakeup(uint64_t id, int level) {
  auto it = std::find_if(tasks.begin(), tasks.end(),
                         [id](const auto &t) { return t->ID() == id; });
  if (it == tasks.end()) {
    return MAKE_ERROR(Error::kNoSuchTask);
  }

  Wakeup(it->get(), level);
  return MAKE_ERROR(Error::kSuccess);
}

Task &TaskManager::CurrentTask() const {
  return *running[current_level].front();
}

Error TaskManager::SendMessage(uint64_t id, const Message &msg) {
  auto it = std::find_if(tasks.begin(), tasks.end(),
                         [id](const auto &t) { return t->ID() == id; });
  if (it == tasks.end()) {
    return MAKE_ERROR(Error::kNoSuchTask);
  }

  (*it)->SendMessage(msg);
  return MAKE_ERROR(Error::kSuccess);
}

void TaskManager::ChangeLevelRunning(Task *task, int level) {
  if (level < 0 || level == task->Level()) {
    return;
  }

  if (task != running[current_level].front()) {
    Erase(running[task->Level()], task);
    running[level].push_front(task);
    task->SetLevel(level);
    if (level > current_level) {
      level_changed = true;
    }
    return;
  }

  running[current_level].pop_front();
  running[level].push_front(task);
  task->SetLevel(level);
  if (level >= current_level) {
    current_level = level;
  } else {
    current_level = level;
    level_changed = true;
  }
}

Task *TaskManager::RotateCurrentRunQueue(bool current_sleep) {
  auto &level_queue = running[current_level];
  Task *current_task = level_queue.front();
  level_queue.pop_front();
  if (!current_sleep) {
    level_queue.push_back(current_task);
  }
  if (level_queue.empty()) {
    level_changed = true;
  }

  if (level_changed) {
    level_changed = false;
    for (int lv = kMaxLevel; lv >= 0; --lv) {
      if (!running[lv].empty()) {
        current_level = lv;
        break;
      }
    }
  }

  return current_task;
}

uint64_t &Task::OSStackPointer() {
  return os_stack_ptr;
}

__attribute__((no_caller_saved_registers)) extern "C" uint64_t
GetCurrentTaskOSStackPointer() {
  return task_manager->CurrentTask().OSStackPointer();
}

uint64_t Task::DPagingBegin() const {
  return dpaging_begin;
}

void Task::SetDPagingBegin(uint64_t v) {
  dpaging_begin = v;
}

uint64_t Task::DPagingEnd() const {
  return dpaging_end;
}

void Task::SetDPagingEnd(uint64_t v) {
  dpaging_end = v;
}
