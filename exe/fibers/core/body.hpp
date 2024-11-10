#pragma once

#include <functional>

namespace exe::fibers {

using Body = std::move_only_function<void()>;

}  // namespace exe::fiber
