#pragma once

#include "../infra/queues/intrusive_queue.hpp"
#include "task/scheduler.hpp"
#include <cstddef>

namespace exe::sched {

class RunLoop : public task::IScheduler {
 public:
  RunLoop() = default;

  RunLoop(const RunLoop&) = delete;
  RunLoop& operator=(const RunLoop&) = delete;

  RunLoop(RunLoop&&) = delete;
  RunLoop& operator=(RunLoop&&) = delete;

  void Submit(task::TaskBase*) override;

  size_t RunAtMost(size_t limit);

  size_t Run();

  bool RunNext();

  bool IsEmpty();

  bool NonEmpty();

 private:
  exe::infra::queues::IntrusiveListSimple<exe::sched::task::TaskBase> queue_;
};

}  // namespace exe::sched