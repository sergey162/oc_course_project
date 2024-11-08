#pragma once

#include "scheduler.hpp"
#include <cstdlib>

namespace exe::sched::task {

template <typename F>
requires requires(F func) { func(); }
inline void Submit(IScheduler& scheduler, F fun) {
  auto ptr = new OtherFunctions(std::move(fun));
  scheduler.Submit(ptr);
}

inline void Submit(IScheduler& scheduler, TaskBase* ptr) {
  scheduler.Submit(ptr);
}

}  // namespace exe::sched::task