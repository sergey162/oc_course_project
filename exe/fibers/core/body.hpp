#pragma once

#include <functional>

namespace exe::fibers {

using Body = std::move_only_function<void()>;  // move-only objects in function

}  // namespace exe::fibers
