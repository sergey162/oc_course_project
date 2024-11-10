#pragma once

#include "../core/scheduler.hpp"
#include "../core/body.hpp"
#include "../core/fiber.hpp"

namespace exe::fibers {

class Fiber;
// Considered harmful

void Go(IScheduler&, Body);

void Go(Body);

}  // namespace exe::fiber