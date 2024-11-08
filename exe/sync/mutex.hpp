#pragma once


#include <mutex> // lock_guard, unique_lock...
#include <atomic>
#include "../../syscalls/futex/wake_wait.hpp"
#include <cstdint>

namespace exe::sync {

class Mutex { // Original Idea - Mutexes are Tricky
 public:

  void Lock() {
    if (TryLock()) { // fast path
      return;
    }
    while (busy_.exchange(2, std::memory_order_acquire)) {
      syscalls::futex::Wait(busy_, 2);
    }
  }

  void Unlock() {
    auto wake_key = syscalls::futex::PrepareWake(busy_);
    std::uint32_t last = busy_.exchange(0, std::memory_order_acq_rel);
    if (last == 2) {
      syscalls::futex::WakeOne(wake_key);
    }
  }

  void unlock() { // NOLINT
    Unlock();
  }

  void lock() { // NOLINT
    Lock();
  }
  
  bool TryLock() {
    uint32_t value = 0;
    return busy_.compare_exchange_weak(value, 1, std::memory_order_acq_rel);
  }

  bool try_lock() { // NOLINT
    return TryLock();
  }

 private:
  std::atomic<uint32_t> busy_ = 0u;
};

}