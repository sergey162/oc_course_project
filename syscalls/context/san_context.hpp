#pragma once

#include <span>

#if __has_feature(address_sanitizer)


#include <sanitizer/asan_interface.h>

namespace syscalls::context {

struct SanitizerContext {
  void Setup(std::span<std::byte> stack) {
    stack_ = stack.data();
    stack_size_ = stack.size();
  }

  void AfterStart() {
    __sanitizer_finish_switch_fiber(nullptr, &(from_->stack_),
                                    &(from_->stack_size_));
  }

  void BeforeSwitch(SanitizerContext& target) {
    target.from_ = this;
    __sanitizer_start_switch_fiber(&fake_stack_, target.stack_,
                                   target.stack_size_);
  }

  void AfterSwitch() {
    __sanitizer_finish_switch_fiber(fake_stack_, &(from_->stack_),
                                    &(from_->stack_size_));
  }

  void BeforeExit(SanitizerContext& target) {
    target.from_ = this;
    // https://github.com/llvm-mirror/compiler-rt/blob/69445f095c22aac2388f939bedebf224a6efcdaf/include/sanitizer/common_interface_defs.h#L299
    __sanitizer_start_switch_fiber(nullptr, target.stack_, target.stack_size_);
  }

 private:
  const void* stack_;
  size_t stack_size_;
  void* fake_stack_;

  SanitizerContext* from_;
};

}  // namespace sure


#elif __has_feature(thread_sanitizer)

#include <sanitizer/tsan_interface.h>

namespace syscalls::context {

struct SanitizerContext {
  void Setup(std::span<std::byte> /*stack*/) {
    fiber_ = __tsan_create_fiber(0);
  }

  void AfterStart() {
    After();
  }

  // NB: __tsan_switch_to_fiber should be called immediately before switch to fiber
  // https://github.com/llvm/llvm-project/blob/712dfec1781db8aa92782b98cac5517db548b7f9/compiler-rt/include/sanitizer/tsan_interface.h#L150-L151
  __attribute__((always_inline))
  inline void BeforeSwitch(SanitizerContext& target) {
    fiber_ = __tsan_get_current_fiber();
    __tsan_switch_to_fiber(target.fiber_, 0);
  }

  void AfterSwitch() {
    After();
  }

  // NB: __tsan_switch_to_fiber should be called immediately before switch to fiber
  __attribute__((always_inline))
  inline void BeforeExit(SanitizerContext& target) {
    target.exit_from_ = this;
    __tsan_switch_to_fiber(target.fiber_, 0);
  }

 private:
  void After() {
    if (exit_from_ != nullptr) {
      __tsan_destroy_fiber(exit_from_->fiber_);
      exit_from_ = nullptr;
    }
  }

 private:
  void* fiber_;
  SanitizerContext* exit_from_{nullptr};
};

}  // namespace sure



#else

namespace syscalls::context {

struct SanitizerContext {
  void Setup(std::span<std::byte>) {
    // Nop
  }

  void AfterStart() {
    // Nop
  }

  void BeforeSwitch(SanitizerContext& /*target*/) {
    // Nop
  }

  void AfterSwitch() {
    // Nop
  }

  void BeforeExit(SanitizerContext& /*target*/) {
    // Nop
  }
};

} 



#endif
