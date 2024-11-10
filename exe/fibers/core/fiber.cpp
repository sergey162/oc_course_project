#include "fiber.hpp"
#include <cstdlib>

thread_local exe::fibers::Fiber* running_fiber = nullptr;

namespace exe::fibers {

Fiber::Fiber(IScheduler& scheduler, Body body)
    : scheduler_(scheduler),
      coro_(std::move(body)) {
}

Fiber::Fiber(IScheduler& scheduler, Body body, size_t stack_size)
    : scheduler_(scheduler),
      coro_(std::move(body), stack_size) {
}

Fiber& Fiber::Self() {
  return *running_fiber;
}

void Fiber::Schedule() {
  exe::sched::task::Submit(scheduler_, this);
}
void Fiber::Run() noexcept {
  Switch();
  if (coro_.IsCompleted()) {
    delete this;
  } else if (awaiter_ != nullptr) {
    std::exchange(awaiter_, nullptr)->RunAwaiter(GetHandler());
  }
}

FiberHandle Fiber::GetHandler() {
  return FiberHandle(this);
}

Fiber* Fiber::SelfPtr() {
  return running_fiber;
}

void Fiber::Switch() {
  Fiber* current_fiber = running_fiber;
  running_fiber = this;
  coro_.Resume();
  running_fiber = current_fiber;
}

void Fiber::Suspend(Awaiter* awaiter) {
  awaiter_ = awaiter;
  coro_.Suspend();
}

IScheduler& Fiber::GetScheduler() {
  return scheduler_;
}

}  // namespace exe::fiber
