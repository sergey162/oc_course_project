#pragma once


#include "wait.hpp"

namespace syscalls::futex {

inline uint32_t Wait(std::atomic_uint32_t& atom, uint32_t old,
                     std::memory_order mo = std::memory_order_seq_cst) {
  return syscalls::atomic::Wait(std::addressof(atom), old, mo);
}

struct [[nodiscard]] WakeKey {
  uint32_t* atom = nullptr;
};

inline WakeKey PrepareWake(std::atomic_uint32_t& atom) {
  return {reinterpret_cast<uint32_t*>(std::addressof(atom))};
}

inline void WakeOne(WakeKey key) {
  syscalls::atomic::WakeOne(key.atom);
}

inline void WakeAll(WakeKey key) {
  syscalls::atomic::WakeAll(reinterpret_cast<unsigned int*>(key.atom));
}


}