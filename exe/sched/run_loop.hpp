#pragma once

#include "task/scheduler.hpp"
#include "../infra/queues/intrusive_queue.hpp"
#include <cstddef>

namespace exe::sched {


class RunLoop : public task::IScheduler {
 public:
  RunLoop() = default;

  // Non-copyable
  RunLoop(const RunLoop&) = delete;
  RunLoop& operator=(const RunLoop&) = delete;

  // Non-movable
  RunLoop(RunLoop&&) = delete;
  RunLoop& operator=(RunLoop&&) = delete;

  // task::IScheduler
  void Submit(task::TaskBase*) override;

  // Run tasks

  size_t RunAtMost(size_t limit);

  size_t Run();

  bool RunNext();

  bool IsEmpty();

  bool NonEmpty();

 private:
  exe::infra::queues::IntrusiveListSimple<exe::sched::task::TaskBase> queue_;
};

}  // namespace exe::sched