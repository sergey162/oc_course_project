#include "coroutine.hpp"

namespace exe::fibers {

Coroutine::Coroutine(Body body)
    : routine_(std::move(body)),
      stack_(Stack::AllocateBytes(10000u)),
      complete_(false) {
  task_context_.Setup(stack_.MutView(), this);
}

Coroutine::Coroutine(Body body, size_t stack_size)
    : routine_(std::move(body)),
      stack_(Stack::AllocateBytes(stack_size)),
      complete_(false) {
  task_context_.Setup(stack_.MutView(), this);
}

void Coroutine::Resume() {
  if (complete_ == false) {
    loop_context_.SwitchTo(task_context_);
  }
}

bool Coroutine::IsCompleted() {
  return complete_;
}

void Coroutine::Suspend() {
  task_context_.SwitchTo(loop_context_);
}

void Coroutine::Run() noexcept {
  routine_();
  complete_ = true;
  task_context_.ExitTo(loop_context_);
}

}  // namespace exe::fibers