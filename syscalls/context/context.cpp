#include "context.hpp"
#include <cstdlib>

namespace syscalls::context {

ExecutionContext::ExecutionContext() {
}

void ExecutionContext::Setup(std::span<std::byte> stack, ITrampoline* trampoline) {
  user_trampoline_ = trampoline;
  machine_.Setup(stack, this);
  sanitizer_.Setup(stack);
}

void ExecutionContext::SwitchTo(ExecutionContext& target) {

  // NB: __tsan_switch_to_fiber should be called immediately before switch to fiber
  // https://github.com/llvm/llvm-project/blob/712dfec1781db8aa92782b98cac5517db548b7f9/compiler-rt/include/sanitizer/tsan_interface.h#L150-L151
  sanitizer_.BeforeSwitch(target.sanitizer_);

  // Switch stacks
  machine_.SwitchTo(target.machine_);

  sanitizer_.AfterSwitch();
}

void ExecutionContext::ExitTo(ExecutionContext& target) {

  // NB: __tsan_switch_to_fiber should be called immediately before switch to fiber
  sanitizer_.BeforeExit(target.sanitizer_);

  // Switch stacks
  machine_.SwitchTo(target.machine_);

}

void ExecutionContext::AfterStart() {
  sanitizer_.AfterStart();
}

void ExecutionContext::Run() noexcept {
  AfterStart();
  user_trampoline_->Run();
}

}  // namespace sure
