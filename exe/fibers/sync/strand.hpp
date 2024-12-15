#pragma once

#include "../../infra/lockfree/lf_stack.hpp"
#include "../../sched/task/submit.hpp"
#include "../../sched/task/task.hpp"
#include "../sched/suspend.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace exe::fibers {

class Strand {
 private:
  struct SimpleAwaiter : public Awaiter,
                         public sched::task::TaskBase {
    SimpleAwaiter(Strand& strand)
        : strand_(strand) {
    }

    void RunAwaiter(FiberHandle handle) noexcept override {
      handle_ = handle;
      exe::sched::task::Submit(handle_.GetScheduler(),
                               static_cast<sched::task::TaskBase*>(this));
    }

    void Run() noexcept override {
      SimpleAwaiter* ptr = Reverse(strand_.stack_.GetChain());
      size_t count = 0;
      while (ptr) {
        auto temp = ptr->next_;
        ptr->RunCallBack();
        ++count;
        ptr->handle_.Schedule();
        ptr = temp;
      }
      size_t old_value = strand_.count_.fetch_sub(count);
      if (count == old_value) {
        handle_.Schedule();
      } else {
        sched::task::Submit(handle_.GetScheduler(),
                            static_cast<sched::task::TaskBase*>(this));
      }
    }

    inline SimpleAwaiter* Reverse(SimpleAwaiter* old_head) {
      if (old_head == nullptr) {
        return nullptr;
      }
      auto next = std::exchange(old_head->next_, nullptr);
      SimpleAwaiter* temp = nullptr;
      while (next != nullptr) {
        temp = next->next_;
        next->next_ = old_head;
        old_head = next;
        next = temp;
      }
      return old_head;
    }

    virtual void RunCallBack() noexcept {
    }

    SimpleAwaiter* next_ = nullptr;
    Strand& strand_;
    FiberHandle handle_;
  };

  template <typename F>
  struct StrandAwaiter : public SimpleAwaiter {
    StrandAwaiter(Strand& strand, F fun)
        : SimpleAwaiter(strand),
          fun_(std::move(fun)) {
    }

    void RunAwaiter(FiberHandle handle) noexcept override {
      handle_ = handle;
      strand_.stack_.Push(static_cast<SimpleAwaiter*>(this));
    }

    void Run() noexcept override {
    }

    void RunCallBack() noexcept override {
      fun_();
    }

    F fun_;
  };

 public:
  template <typename U>
  friend struct StrandAwaiter;

  friend struct SimpleAwaiter;

 public:
  Strand() = default;

  Strand(const Strand&) = delete;

  Strand(Strand&&) = delete;

  Strand& operator=(const Strand&) = delete;

  Strand& operator=(Strand&&) = delete;

  template <typename F>
  void Combine(F func) {
    if (count_.fetch_add(1) == 0) {
      func();
      if (count_.fetch_sub(1) == 1) {
        return;
      }
      SimpleAwaiter awaiter(*this);
      Suspend(&awaiter);
    } else {
      StrandAwaiter awaiter(*this, std::move(func));
      Suspend(&awaiter);
    }
  }

  ~Strand() = default;

 private:
  exe::infra::lockfree::LockFreeStack2<SimpleAwaiter> stack_;
  std::atomic<size_t> count_ = 0u;
};

}  // namespace exe::fibers
