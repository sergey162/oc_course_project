#pragma once

#include <cstddef>

#include <memory>
#include "../../sync/spin_lock.hpp"
#include "../../infra/queues/ring_queue.hpp"
#include "../sched/suspend.hpp"
#include "../../sched/task/task.hpp"
#include "../../infra/queues/intrusive_queue.hpp"


namespace exe::fibers {

namespace detail {

// State shared between producers & consumers

template <typename T>
class BufferedChannelState {

 private:

  struct TempAwaiter : public Awaiter, public sched::task::TaskBase {};
  
  struct SenderAwaiter : public TempAwaiter {

    SenderAwaiter(BufferedChannelState& state, T& value) : state_(state), value_(value) {}

    void RunAwaiter(FiberHandle handle) noexcept override {
      handler_ = handle;
      state_.wait_queue_.Push(static_cast<sched::task::TaskBase*>(this));
      state_.spin_lock_.unlock();
    }

    void Run() noexcept override {}

    BufferedChannelState& state_;
    T& value_;
    FiberHandle handler_;
  };

  struct ReceiverAwaiter : public TempAwaiter {

    ReceiverAwaiter(BufferedChannelState& state, std::optional<T>& value) : state_(state), value_(value) {}

    void RunAwaiter(FiberHandle handle) noexcept override {
      handler_ = handle;
      state_.wait_queue_.Push(static_cast<sched::task::TaskBase*>(this));
      state_.spin_lock_.unlock();
    }

    void Run() noexcept override {}

    BufferedChannelState& state_;
    std::optional<T>& value_;
    FiberHandle handler_;
  };

 public:
  explicit BufferedChannelState(size_t capacity)
      : queue_(capacity) {
  }

  ~BufferedChannelState() {
  }

  void Send(T value) {
    spin_lock_.lock();
    if (queue_.IsEmpty()) {
      if (wait_queue_.IsEmpty()) {
        queue_.TryPush(std::move(value));
        spin_lock_.unlock();
      } else {
        ReceiverAwaiter* node = static_cast<ReceiverAwaiter*>(wait_queue_.Pop());
        node->value_.emplace(std::move(value));
        spin_lock_.unlock();
        node->handler_.Schedule();
      }
    } else if (queue_.IsFull()) {
      SenderAwaiter awaiter(*this, value);
      Suspend(&awaiter);
    } else {
      queue_.TryPush(std::move(value));
      spin_lock_.unlock();
    }

  }

  T Recv() {
    spin_lock_.lock();
    std::optional<T> value = queue_.TryPop();
    if (!value.has_value()) {
      ReceiverAwaiter awaiter(*this, value);
      Suspend(&awaiter);
    } else {
      if (!wait_queue_.IsEmpty()) {
        SenderAwaiter* node = static_cast<SenderAwaiter*>(wait_queue_.Pop());
        queue_.TryPush(std::move(node->value_));
        spin_lock_.unlock();
        node->handler_.Schedule();
      } else {
        spin_lock_.unlock();
      }
    }
    return std::move(value.value());
  }

 private:
  exe::sync::SpinLock spin_lock_;
  exe::infra::queues::IntrusiveListSimple<exe::sched::task::TaskBase> wait_queue_;
  exe::infra::queues::RingQueue<T> queue_;
};

}  // namespace detail


template <typename T>
class BufferedChannel {
  using State = detail::BufferedChannelState<T>;

 public:
  // Bounded channel, `capacity` > 0
  explicit BufferedChannel(size_t capacity)
      : state_(std::make_shared<State>(capacity)) {
  }

  void Send(T value) {
    state_->Send(std::move(value));
  }

  T Recv() {
    return state_->Recv();
  }

 private:
  std::shared_ptr<State> state_;
};

}  // namespace exe::fiber

