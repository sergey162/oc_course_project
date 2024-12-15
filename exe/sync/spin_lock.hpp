#pragma once
#include <atomic>

namespace exe::sync {

class SpinLock {
 public:
  void Lock() {
    while (locked_.exchange(true)) {
      while (locked_.load()) {
      }
    }
  }

  bool TryLock() {
    return !locked_.exchange(true);
  }

  void Unlock() {
    locked_.store(false);
  }

  // Lockable

  void lock() {  // NOLINT
    Lock();
  }

  bool try_lock() {  // NOLINT
    return TryLock();
  }

  void unlock() {  // NOLINT
    Unlock();
  }

 private:
  std::atomic<bool> locked_{false};
};

}  // namespace exe::sync
