#include "exe/sched/thread_pool.hpp"
#include "exe/sched/task/submit.hpp"
#include <iostream>

int main() {
  exe::sched::ThreadPool pool{4};
  for (size_t _ = 0; _ < 100; ++_) {
    exe::sched::task::Submit(pool, []{std::cout << 1 << std::endl;});
  }
  pool.Start();
  pool.Stop();
  return 0;
}