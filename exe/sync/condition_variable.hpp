#pragma once

#include "mutex.hpp"

namespace exe::sync {

class CondVar {
 public:
  template <typename Mutex>
  void Wait(Mutex& other) {
    std::uint32_t value = count_.load();
    other.unlock();
    syscalls::futex::Wait(count_, value);
    other.lock();
  }

  void NotifyOne() {
    auto wake_key = syscalls::futex::PrepareWake(count_);
    count_.fetch_add(1);
    syscalls::futex::WakeOne(wake_key);
  }

  void NotifyAll() {
    auto wake_key = syscalls::futex::PrepareWake(count_);
    count_.fetch_add(1);
    syscalls::futex::WakeAll(wake_key);
  }

 private:
  std::atomic<uint32_t> count_{0u};
};

}  // namespace exe::sync