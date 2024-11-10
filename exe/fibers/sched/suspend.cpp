#include "suspend.hpp"

namespace exe::fibers {

void Suspend(Awaiter* awaiter) {
  if (Fiber::SelfPtr() != nullptr) {
    Fiber::Self().Suspend(awaiter);
  }
}

}  // namespace exe::fiber