#pragma once

#include "../../infra/lockfree/lf_queue.hpp"
#include "../sched/suspend.hpp"
#include <cstdint>
#include <atomic>

namespace exe::fibers {

class Mutex {
 private:
  class MutexAwaiter : public Awaiter {
   public:
    MutexAwaiter(Mutex& mutex) : mutex_(mutex) {}

    void RunAwaiter(FiberHandle handle) noexcept override {
      handle_ = handle;
      mutex_.queue_.Push(this); // no alloc
    }
    virtual ~MutexAwaiter() = default;

    friend class Mutex;

   public:
    MutexAwaiter* next_ = nullptr;

   private:
    FiberHandle handle_;
    Mutex& mutex_;
  };

 public:
  void Lock() {
    if (working_.fetch_add(1) == 0) {
      return;
    }
    MutexAwaiter awaiter(*this);
    Suspend(&awaiter);
  }

  void Unlock() {
    if (working_.fetch_sub(1) != 1) {
      while (true) {
        MutexAwaiter* node = queue_.TryPop();
        if (node != nullptr) {
          node->handle_.Schedule();
          return;
        }
      }
    }
  }

  void lock() {  // NOLINT
    Lock();
  }

  bool try_lock() {  // NOLINT
    return TryLock();
  }

  bool TryLock() {
    uint32_t value = 0u;
    return working_.compare_exchange_strong(value, 1u);
  }

  void unlock() {
    Unlock();
  }

 private:
  std::atomic<uint32_t> working_ = 0u;
  exe::infra::lockfree::LockFreeQueue2<MutexAwaiter> queue_;
};

}  // namespace exe::fiber