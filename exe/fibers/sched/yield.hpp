#pragma once

#include "../core/fiber.hpp"
#include "suspend.hpp"

namespace exe::fibers {

class YieldAwaiter : public Awaiter {
 public:
  void RunAwaiter(FiberHandle) noexcept override;
  ~YieldAwaiter() override = default;
};

void Yield();

}  // namespace exe::fibers
