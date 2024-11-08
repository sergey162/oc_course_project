#pragma once


#include "program/prog_context.hpp"
#include "san_context.hpp"
#include <span>

namespace syscalls::context {

struct ITrampoline {
  virtual void Run() noexcept = 0;

 protected:
  ~ITrampoline() = default;
};


// Execution Context =
// 1) Machine context (registers) +
// 2) [Address | Thread] sanitizer context +

class ExecutionContext final : private ITrampoline {
 public:

  ExecutionContext();

  ExecutionContext(const ExecutionContext&) = delete;
  ExecutionContext& operator=(const ExecutionContext&) = delete;

  ExecutionContext(ExecutionContext&&) = delete;
  ExecutionContext& operator=(ExecutionContext&&) = delete;


  void Setup(std::span<std::byte> stack, ITrampoline* trampoline);

  // Symmetric Control Transfer
  // 1) Save current execution context to `this`
  // 2) Activate `target` context
  void SwitchTo(ExecutionContext& target);

  // Leave current execution context forever
  // Last context switch in ITrampoline::Run
  void ExitTo(ExecutionContext& target);

 private:
  // ITrampoline
  void Run() noexcept override;

  // Finalize first context switch
  void AfterStart();

 private:
  ITrampoline* user_trampoline_;
  MachineContext machine_;
  SanitizerContext sanitizer_;
};

}  // namespace sure
