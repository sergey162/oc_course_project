#pragma once

#include <type_traits>

namespace exe::infra::queues {

template <typename T>
  requires requires(T* ptr) {
    requires(std::is_same_v<std::decay_t<decltype(ptr->next_node_)>, T*>);
  }
class IntrusiveListSimple {
 public:
  IntrusiveListSimple() = default;

  void Push(T* ptr) {
    if (IsClose()) {
      return;
    }
    ptr->next_node_ = nullptr;
    if (size_ == 0) {
      head_ = ptr;
    } else {
      tail_->next_node_ = ptr;
    }
    tail_ = ptr;
    ++size_;
  }

  T* Pop() {
    if (size_ == 0) {
      return nullptr;
    }
    T* val = head_;
    head_ = head_->next_node_;
    if (head_ == nullptr) {
      tail_ = nullptr;
    }
    val->next_node_ = nullptr;
    --size_;
    return val;
  }

  void Close() {
    availible_ = false;
  }

  bool IsClose() {
    return !availible_;
  }

  bool IsEmpty() {
    return size_ == 0;
  }

  std::size_t Size() {
    return size_;
  }

  ~IntrusiveListSimple() = default;

 private:
  T* head_ = nullptr;
  T* tail_ = nullptr;
  std::size_t size_ = 0u;
  bool availible_ = true;
};

}  // namespace exe::infra::queues