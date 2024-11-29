#pragma once



#include <cstdint>
#include <atomic>
#include "../sched/suspend.hpp"
#include "../../infra/lockfree/lf_queue.hpp"

namespace exe::fibers {

struct Stamped {
  uint32_t waiters_ = 0u; 
  uint32_t count_ = 0u;
  explicit operator uint64_t() const noexcept {
    return (static_cast<uint64_t>(waiters_) << 32u) + static_cast<uint64_t>(count_);
  }
  static Stamped to_stamped(uint64_t value) noexcept {
    return {static_cast<uint32_t>(value >> 32u), static_cast<uint32_t>(value)};
  }
};


class WaitGroup {
 private:

  struct WGAwaiter : public Awaiter {
    WGAwaiter(WaitGroup& wg) : wg_(wg) {
    }

    void RunAwaiter(FiberHandle handle) noexcept override {
      handle_ = handle;
      wg_.queue_.Push(this);
    }

    WGAwaiter* next_ = nullptr; 
    FiberHandle handle_;
    WaitGroup& wg_;                         
  };

  friend struct WGAwaiter;

 public:
  WaitGroup() = default;

  WaitGroup(const WaitGroup&) = delete;

  WaitGroup(WaitGroup&&) = delete;

  WaitGroup& operator=(const WaitGroup&) = delete;

  WaitGroup& operator=(WaitGroup&&) = delete;

  void Add(size_t count) {
    count_.fetch_add(static_cast<uint64_t>(count), std::memory_order_release);
  }

  void Done() {
    Stamped current_stamped{0, 0};
    uint32_t waiters = 0u;
    while (true) {
      uint64_t current = count_.load(std::memory_order_relaxed);
      current_stamped = Stamped::to_stamped(current);
      if (--current_stamped.count_ == 0u && current_stamped.waiters_ != 0u) {
        waiters = std::exchange(current_stamped.waiters_, 0u);
        if (count_.compare_exchange_weak(current, static_cast<uint64_t>(current_stamped), std::memory_order_release, std::memory_order_relaxed)) {
          break;
        }
      } else {
        if (count_.compare_exchange_weak(current, static_cast<uint64_t>(current_stamped), std::memory_order_release, std::memory_order_relaxed)) {
          return;
        }
      }
    }
    WGAwaiter* node = nullptr;
    WGAwaiter* current = nullptr;
    while (true) {
      WGAwaiter* ptr = queue_.TryPop();
      if (ptr == nullptr) {
        continue;
      }
      ptr->next_ = nullptr;
      if (node == nullptr) {
        current = ptr;
        node = ptr;
      } else {
        current->next_ = ptr;
        current = ptr;
      }
      if (--waiters == 0u) {
        break;
      }
    }
    while (node) {
      WGAwaiter* next = node->next_;
      node->handle_.Schedule();
      node = next;
    }
  }
  
  void Wait() {
    while (true) {
      uint64_t current = count_.load(std::memory_order_acquire);
      Stamped current_stamped = Stamped::to_stamped(current);
      if (current_stamped.count_ == 0u) {
        return;
      }
      ++current_stamped.waiters_;
      if (count_.compare_exchange_weak(current, static_cast<uint64_t>(current_stamped), std::memory_order_release, std::memory_order_relaxed)) {
        WGAwaiter awaiter(*this);
        Suspend(&awaiter);
      }
    }
  }

  ~WaitGroup() = default;

 private:
  std::atomic<uint64_t> count_ = 0u;
  exe::infra::lockfree::LockFreeQueue2<WGAwaiter> queue_;
};

}  // namespace exe::fiber