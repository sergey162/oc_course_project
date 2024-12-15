#pragma once

#include <atomic>
#include <cstdint>
#include <linux/futex.h>
#include <sys/syscall.h>

extern "C" {

long syscall(long number, ...);
}

namespace syscalls::atomic {

inline int Wait(unsigned int* loc, std::uint32_t old) {
  return syscall(SYS_futex, loc, FUTEX_WAIT_PRIVATE, old, nullptr, nullptr, 0);
}

inline std::uint32_t Wait(std::atomic_uint32_t* value, std::uint32_t old,
                          std::memory_order mo) {
  std::uint32_t current_value = 0;
  do {
    Wait(reinterpret_cast<unsigned int*>(value), old);
    current_value = value->load(mo);
  } while (current_value == old);
  return current_value;
}

inline int WakeOne(unsigned int* loc) {
  return syscall(SYS_futex, loc, FUTEX_WAKE_PRIVATE, 1, nullptr, nullptr, 0);
}

inline int WakeAll(unsigned int* loc) {
  return syscall(SYS_futex, loc, FUTEX_WAKE_PRIVATE, INT32_MAX, nullptr,
                 nullptr, 0);
}

}  // namespace syscalls::atomic