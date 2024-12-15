#include "run_loop.hpp"

namespace exe::sched {

void RunLoop::Submit(task::TaskBase* task) {
  queue_.Push(task);
}

// Run tasks

size_t RunLoop::RunAtMost(size_t limit) {
  size_t value = 0;
  while (value < limit && NonEmpty()) {
    task::TaskBase* task = queue_.Pop();
    task->Run();
    ++value;
  }
  return value;
}

size_t RunLoop::Run() {
  std::size_t count = 0;
  while (queue_.Size() != 0u) {
    task::TaskBase* task = queue_.Pop();
    task->Run();
    ++count;
  }
  return count;
}

bool RunLoop::RunNext() {
  return RunAtMost(1) == 1;
}

bool RunLoop::IsEmpty() {
  return queue_.IsEmpty();
}

bool RunLoop::NonEmpty() {
  return !IsEmpty();
}

}  // namespace exe::sched
