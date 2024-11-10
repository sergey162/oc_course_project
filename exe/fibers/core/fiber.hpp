#pragma once

#include "coroutine.hpp"
#include "scheduler.hpp"
#include "awaiter.hpp"
#include "handle.hpp"
#include <utility>
#include "../../sched/task/submit.hpp"

namespace exe::fibers {

class Fiber : public exe::sched::task::TaskBase {
 public:
  Fiber(IScheduler& scheduler, Body body);

  Fiber(IScheduler& scheduler, Body body, size_t stack_size);

  void Suspend(Awaiter* awaiter);

  void Schedule();

  void Switch();

  void Run() noexcept override;

  FiberHandle GetHandler();

  static Fiber& Self();

  static Fiber* SelfPtr();

  IScheduler& GetScheduler();

  virtual ~Fiber() = default;

 private:
  IScheduler& scheduler_;
  Awaiter* awaiter_ = nullptr;
  Coroutine coro_;
};

}  // namespace exe::fiber