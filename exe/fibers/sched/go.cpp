#include "go.hpp"

namespace exe::fibers {

void Go(IScheduler& other, Body func) {
  Fiber* new_fiber = new Fiber(other, std::move(func));
  sched::task::Submit(other, new_fiber);
}

void Go(Body func) {
  IScheduler& sched = Fiber::Self().GetScheduler();
  Fiber* new_fiber = new Fiber(sched, std::move(func));
  sched::task::Submit(sched, new_fiber);
}

}  // namespace exe::fiber