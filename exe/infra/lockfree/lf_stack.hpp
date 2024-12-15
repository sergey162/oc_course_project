#pragma once

#include <atomic>
#include <cstdint>
#include <optional>

namespace exe::infra::lockfree {

template <typename T>
class LockFreeStack2 {
 public:
  LockFreeStack2()
      : head_(nullptr) {
  }

  void Push(T* node) {
    while (true) {
      T* current_head = head_.load();
      node->next_ = current_head;
      if (head_.compare_exchange_weak(current_head, node)) {
        return;
      }
    }
  }

  bool TryPush(T* node) {
    while (true) {
      T* current_head = head_.load();
      if ((T*)1 == current_head) {
        return false;
      }
      node->next_ = current_head;
      if (head_.compare_exchange_weak(current_head, node)) {
        return true;
      }
    }
  }

  T* TryPop() {
    while (true) {
      T* current_head = head_.load();
      if (current_head == nullptr) {
        return nullptr;
      }
      if (head_.compare_exchange_weak(current_head, current_head->next_)) {
        return current_head;
      }
    }
  }

  bool IsInvalidate() {
    return ((T*)1 == head_.load());
  }

  T* GetChain() {
    return head_.exchange(nullptr);
  }

  T* GetChainAndInvalidate() {
    auto head = head_.exchange((T*)1);
    if (head == (T*)1)
      return nullptr;
    return head;
  }

 private:
  std::atomic<T*> head_;
};

}  // namespace exe::infra::lockfree