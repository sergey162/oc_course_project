#pragma once

#include <cstddef>
#include "mutex.hpp"
#include "condition_variable.hpp"


namespace exe::sync {

class WaitGroup {
 public:

  void Add(size_t count) {
    std::lock_guard lock(mutex_);
    count_workers_ += count;
  }

  void Done() {
    std::lock_guard lock(mutex_);
    if (--count_workers_ == 0 && waiters_ != 0) {
      cv_.NotifyAll();
      waiters_ = 0;
    }
  }

  void Wait() {
    mutex_.Lock();
    if (count_workers_ == 0) {
    } else {
      ++waiters_;
      while (count_workers_ != 0) {
        cv_.Wait(mutex_);
      }
    }
    mutex_.Unlock();
  }

 private:
  Mutex mutex_;
  CondVar cv_;
  std::size_t count_workers_ = 0;
  std::size_t waiters_ = 0;
};

}
