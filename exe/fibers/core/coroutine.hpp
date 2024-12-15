#pragma once

#include "stack.hpp"

#include "../../../syscalls/context/context.hpp"

#include "body.hpp"

namespace exe::fibers {

class Coroutine : private syscalls::context::ITrampoline {
 public:
  explicit Coroutine(Body body);

  Coroutine(Body body, size_t stack_size);

  void Resume();

  bool IsCompleted();

  ~Coroutine() = default;

  void Suspend();

 private:
  void Run() noexcept override;

 private:
  Body routine_;
  syscalls::context::ExecutionContext loop_context_;
  syscalls::context::ExecutionContext task_context_;
  Stack stack_;
  bool complete_ = false;
};

}  // namespace exe::fibers
