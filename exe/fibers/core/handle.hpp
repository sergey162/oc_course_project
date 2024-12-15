#pragma once

#include "fwd.hpp"

namespace exe::fibers {

class FiberHandle {
  friend class Fiber;

 public:
  FiberHandle(const FiberHandle& other)
      : fiber_(other.fiber_) {
  }

  FiberHandle(FiberHandle&& other)
      : fiber_(other.fiber_) {
  }

  FiberHandle& operator=(FiberHandle&&) = default;

  FiberHandle& operator=(const FiberHandle&) = default;

  ~FiberHandle() = default;

  FiberHandle()
      : FiberHandle(nullptr) {
  }

  static FiberHandle Invalid() {
    return FiberHandle(nullptr);
  }

  bool IsValid() const {
    return fiber_ != nullptr;
  }

  // Schedule fiber to the associated scheduler
  void Schedule();

  // Switch to this fiber immediately
  // For symmetric transfer
  void Switch();

  exe::sched::task::IScheduler& GetScheduler() noexcept;

  explicit FiberHandle(Fiber* fiber)
      : fiber_(fiber) {
  }

 private:
  Fiber* Release();

  Fiber* fiber_ = nullptr;
};

}  // namespace exe::fibers