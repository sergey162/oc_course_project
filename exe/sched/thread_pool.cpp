#include "thread_pool.hpp"


thread_local exe::sched::ThreadPool* g_poolPtr = nullptr;

namespace exe::sched {

ThreadPool::ThreadPool(size_t threads)
    : threads_(threads) {
  workers_.reserve(threads);
}

void ThreadPool::Start() {
  for (size_t i = 0; i < threads_; ++i) {
    workers_.emplace_back([this] {
      g_poolPtr = this;
      WorkerRoutine();
    });
  }
}

ThreadPool::~ThreadPool() {
}

void ThreadPool::Submit(task::TaskBase* task) {
  wg_.Add(1);
  queue_.Push(task);
}

ThreadPool* ThreadPool::Current() {
  return g_poolPtr;
}

void ThreadPool::Stop() {
  wg_.Wait();
  queue_.Close();
  for (auto& thread : workers_) {
    thread.join();
  }
}

void ThreadPool::WorkerRoutine() {
  while (true) {
    task::TaskBase* task = queue_.Pop();
    if (task == nullptr) { // queue closed
      return;
    } else {
      task->Run();
      wg_.Done();
    }
  }
}
}  // namespace exe::sched
