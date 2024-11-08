#pragma once


#include "task/scheduler.hpp"
#include "../infra/queues/intr_mpmc_block_queue.hpp"
#include "../sync/wait_group.hpp"
#include <thread>
#include <cstddef>
#include <vector>
#include <utility>

namespace exe::sched {

class ThreadPool : public task::IScheduler {
 public:
  explicit ThreadPool(size_t threads);
  ~ThreadPool();

  // Non-copyable
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Non-movable
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  void Start();

  void Submit(task::TaskBase* task) override;

  static ThreadPool* Current();

  void Stop();

 private:
  void WorkerRoutine();
  size_t threads_;
  std::vector<std::thread> workers_;
  exe::infra::queues::IntrusiveMPMCBlockingQueue<exe::sched::task::TaskBase> queue_;
  exe::sync::WaitGroup wg_;
};

}  // namespace exe::sched