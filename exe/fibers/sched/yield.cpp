#include "yield.hpp"

namespace exe::fibers {

void YieldAwaiter::RunAwaiter(FiberHandle handle) noexcept {
  handle.Schedule();
}

void Yield() {
  YieldAwaiter awaiter;
  exe::fibers::Suspend(&awaiter);
}

}  // namespace exe::fibers
