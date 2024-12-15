#pragma once

#include "../core/body.hpp"
#include "../core/fiber.hpp"
#include "../core/scheduler.hpp"

namespace exe::fibers {

class Fiber;

void Go(IScheduler&, Body);

void Go(Body);

}  // namespace exe::fibers