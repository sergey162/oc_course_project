#pragma once


#include "../../sync/condition_variable.hpp"
#include <iostream>
#include "../../sync/mutex.hpp"
#include <atomic>
#include <type_traits>


namespace exe::infra::queues {
template <typename T>
requires requires (T* ptr) {
  requires(std::is_same_v<std::decay_t<decltype(ptr->next_node_)>, T*>);
}
class IntrusiveMPMCBlockingQueue {
 public:

  IntrusiveMPMCBlockingQueue() = default;

  void Push(T* ptr) {
    {
      std::lock_guard lock(mutex_);
      ptr->next_node_ = nullptr;
      if (size_ == 0) {
        head_ = ptr;
      } else {
        tail_->next_node_ = ptr;
      }
      tail_ = ptr;
      ++size_;
    }
    cv_.NotifyOne();
  }

  T* Pop() {
    mutex_.Lock();
    while (size_ == 0) {
      if (!availible_) {
        mutex_.Unlock();
        return nullptr;
      }
      cv_.Wait(mutex_);
    }
    T* val = head_;
    head_ = head_->next_node_;
    if (head_ == nullptr) {
      tail_ = nullptr;
    }
    val->next_node_ = nullptr;
    --size_;
    mutex_.Unlock();
    return val;
  }

  void Close() {
    std::lock_guard lock(mutex_);
    availible_ = false;
    cv_.NotifyAll();
  }

  bool IsClose() {
    std::lock_guard lock(mutex_);
    return !availible_;
  }

  bool IsEmpty() {
    std::lock_guard lock(mutex_);
    return size_ == 0;
  }

  std::size_t Size() {
    std::lock_guard lock(mutex_);
    return size_;
  }

  ~IntrusiveMPMCBlockingQueue() = default;

 private:
  T* head_ = nullptr;
  T* tail_ = nullptr;
  std::size_t size_ = 0u;
  bool availible_ = true;
  exe::sync::Mutex mutex_;
  exe::sync::CondVar cv_;
};

}
