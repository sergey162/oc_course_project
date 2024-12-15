#pragma once

#include "lf_stack.hpp"

namespace exe::infra::lockfree {

template <typename T>
class LockFreeQueue2 {
 public:
  void Push(T* node) {
    second_.Push(node);
  }

  T* TryPop() {
    auto value = first_.TryPop();
    if (value != nullptr) {
      return value;
    }
    while (true) {
      T* value = second_.TryPop();
      if (value != nullptr) {
        first_.Push(value);
      } else {
        break;
      }
    }
    return first_.TryPop();
  };

 private:
  LockFreeStack2<T> first_;
  LockFreeStack2<T> second_;
};

}  // namespace exe::infra::lockfree