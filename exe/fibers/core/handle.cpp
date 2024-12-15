#include "handle.hpp"
#include "fiber.hpp"
#include <utility>

namespace exe::fibers {

Fiber* FiberHandle::Release() {
  return std::exchange(fiber_, nullptr);
}

void FiberHandle::Schedule() {
  fiber_->Schedule();
}

void FiberHandle::Switch() {
  fiber_->Switch();
}

exe::sched::task::IScheduler& FiberHandle::GetScheduler() noexcept {
  return fiber_->GetScheduler();
}

}  // namespace exe::fibers
