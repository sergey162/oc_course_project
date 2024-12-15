#pragma once

#include <optional>
#include <vector>

namespace exe::infra::queues {

template <typename Task>
class RingQueue {
 public:
  RingQueue(size_t cap)
      : capacity_(cap),
        buffer_(cap) {
  }

  RingQueue(const RingQueue&) = delete;

  RingQueue(RingQueue&&) = delete;

  RingQueue& operator=(const RingQueue&) = delete;

  RingQueue& operator=(RingQueue&&) = delete;

  ~RingQueue() = default;

  bool TryPush(Task task) {
    if (tail_ - head_ == capacity_ + 1) {
      return false;
    }
    buffer_[(tail_ - 1) % capacity_].emplace(std::move(task));
    ++tail_;
    return true;
  }

  std::optional<Task> TryPop() {
    if (head_ + 1 == tail_) {
      return std::nullopt;
    }
    Task task = std::move(buffer_[head_ % capacity_].value());
    buffer_[head_ % capacity_].reset();
    ++head_;
    return std::move(task);
  }

  bool IsEmpty() const noexcept {
    return (tail_ - head_ - 1 == 0);
  }

  bool IsFull() const noexcept {
    return (tail_ - head_ - 1 == capacity_);
  }

 private:
  const size_t capacity_;
  std::vector<std::optional<Task>> buffer_;
  size_t head_ = 0u;
  size_t tail_ = 1u;
};

}  // namespace exe::infra::queues