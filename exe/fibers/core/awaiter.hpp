#pragma once

#include "handle.hpp"

namespace exe::fibers {

class Awaiter {
 public:
  virtual void RunAwaiter(FiberHandle) noexcept = 0;

  virtual ~Awaiter() = default;
};


}  // namespace exe::fiber
