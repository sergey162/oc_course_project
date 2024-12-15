#pragma once

#include <span>

extern "C" {

void* SetupMachineContext(void* stack, void* trampoline, void* arg);

void SwitchMachineContext(void** from_rsp, void** to_rsp);
}

namespace syscalls::context {

static void MachineContextTrampoline(void*, void*, void*, void*, void*, void*,
                                     void* arg7) {
  ITrampoline* t = (ITrampoline*)arg7;
  t->Run();
}

// Target architecture: x86-64

struct MachineContext {
  void* rsp_;

  void Setup(std::span<std::byte> stack, ITrampoline* trampoline) {
    rsp_ =
        SetupMachineContext((void*)&stack.back(),
                            (void*)MachineContextTrampoline, (void*)trampoline);
  }

  void SwitchTo(MachineContext& target) {
    SwitchMachineContext(&rsp_, &target.rsp_);
  }

  static constexpr bool kStackPointerAvailable = true;

  void* StackPointer() const noexcept {
    return rsp_;
  }
};

}  // namespace syscalls::context
