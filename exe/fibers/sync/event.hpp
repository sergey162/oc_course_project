#pragma once


#include "../../infra/lockfree/lf_stack.hpp"
#include "../sched/suspend.hpp"
#include <cstdint>

namespace exe::fibers {

class Event {
 private:
  class EventAwaiter : public Awaiter {
   public:
    EventAwaiter() = delete;
    EventAwaiter(Event& event) : event_(event) {}

    void RunAwaiter(FiberHandle handle) noexcept override {
      handle_ = handle;
      if (!event_.stack_.TryPush(this)) { //  if stack close - fail
        handle.Schedule();
      }
    }
   public:
    friend class Event;
    EventAwaiter* next_ = nullptr;
   private:
    Event& event_;
    FiberHandle handle_;
  };

  friend class EventAwaiter;
 public:
  void Wait() {
    if (stack_.IsInvalidate()) {
      return;
    }
    EventAwaiter awaiter(*this);
    Suspend(&awaiter);
  }
  
  void Fire() {
    EventAwaiter* current = stack_.GetChainAndInvalidate();
    while (current != nullptr) {
      EventAwaiter* next = current->next_;
      current->handle_.Schedule();
      current = next;
    }
  }

 private:
  exe::infra::lockfree::LockFreeStack2<EventAwaiter> stack_;
};

}