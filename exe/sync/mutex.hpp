#pragma once

#include "../../syscalls/futex/wake_wait.hpp"
#include <atomic>
#include <cstdint>
#include <mutex>

namespace exe::sync {

class Mutex {
 public:
  void Lock() {
    if (TryLock()) {
      return;
    }
    while (busy_.exchange(2)) {
      syscalls::futex::Wait(busy_, 2);
    }
  }

  void Unlock() {
    auto wake_key = syscalls::futex::PrepareWake(busy_);
    std::uint32_t last = busy_.exchange(0);
    if (last == 2) {
      syscalls::futex::WakeOne(wake_key);
    }
  }

  void unlock() {  // NOLINT
    Unlock();
  }

  void lock() {  // NOLINT
    Lock();
  }

  bool TryLock() {
    uint32_t value = 0;
    return busy_.compare_exchange_strong(value, 1);
  }

  bool try_lock() {  // NOLINT
    return TryLock();
  }

 private:
  std::atomic<uint32_t> busy_ = 0u;
};

}  // namespace exe::sync