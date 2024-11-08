#pragma once

#include <utility>

namespace exe::sched::task {

struct ITask {
  virtual void Run() noexcept = 0;

 protected:
  ~ITask() = default;
};

// Intrusive task

struct TaskBase : public ITask {
  TaskBase* next_node_ = nullptr;
};

template <typename F>
class OtherFunctions : public TaskBase {  // Not fibers
 public:
  explicit OtherFunctions(F fun)
      : fun_(std::move(fun)) {
  }

  void Run() noexcept override {
    fun_();
    delete this;
  }

  virtual ~OtherFunctions() = default;

 private:
  F fun_;
};

}  // namespace exe::sched::task