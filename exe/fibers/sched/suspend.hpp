#pragma once

#include "../core/fiber.hpp"

#include "../core/awaiter.hpp"

namespace exe::fibers {

void Suspend(Awaiter* awaiter);

}  // namespace exe::fibers
