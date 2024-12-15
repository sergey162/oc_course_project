#include "../exe/fibers/core/fiber.hpp"
#include "../exe/fibers/sched/go.hpp"
#include "../exe/fibers/sched/yield.hpp"
#include "../exe/fibers/sync/buffered_channel.hpp"
#include "../exe/fibers/sync/event.hpp"
#include "../exe/fibers/sync/mutex.hpp"
#include "../exe/fibers/sync/strand.hpp"
#include "../exe/fibers/sync/wait_group.hpp"
#include "../exe/sched/run_loop.hpp"
#include "../exe/sched/thread_pool.hpp"
#include <gtest/gtest.h>

using namespace exe;

TEST(Fibers, Go) {
  exe::sched::RunLoop loop;

  exe::fibers::Go(loop, [] {});

  size_t tasks = loop.Run();
  ASSERT_EQ(tasks, 1);
}

TEST(Fibers, GoGroup) {
  exe::sched::RunLoop loop;

  const size_t kFibers = 7;

  for (size_t i = 0; i < kFibers; ++i) {
    exe::fibers::Go(loop, [] {});
  }

  size_t tasks = loop.Run();
  ASSERT_EQ(tasks, kFibers);
}

TEST(Fibers, GoChild) {
  exe::sched::RunLoop loop;

  bool flag = false;

  exe::fibers::Go(loop, [&] {
    exe::fibers::Go([&] {
      flag = true;
    });
  });

  ASSERT_TRUE(loop.RunNext());
  ASSERT_FALSE(flag);
  ASSERT_EQ(loop.Run(), 1);
  ASSERT_TRUE(flag);
}

TEST(Fibers, Yield) {
  exe::sched::RunLoop loop;

  exe::fibers::Go(loop, [] {
    exe::fibers::Yield();
  });

  ASSERT_EQ(loop.Run(), 2);
}

TEST(Fibers, PingPong) {
  exe::sched::RunLoop loop;

  int turn = 0;

  exe::fibers::Go(loop, [&] {
    for (size_t i = 0; i < 3; ++i) {
      ASSERT_EQ(turn, 0);
      turn ^= 1;
      exe::fibers::Yield();
    }
  });

  exe::fibers::Go(loop, [&] {
    for (size_t j = 0; j < 3; ++j) {
      ASSERT_EQ(turn, 1);
      turn ^= 1;
      exe::fibers::Yield();
    }
  });

  loop.Run();
}

TEST(Fibers, YieldGroup) {
  exe::sched::RunLoop loop;

  const size_t kFibers = 3;
  const size_t kYields = 4;

  for (size_t i = 0; i < kFibers; ++i) {
    exe::fibers::Go(loop, [] {
      for (size_t k = 0; k < kYields; ++k) {
        exe::fibers::Yield();
      }
    });
  }

  size_t tasks = loop.Run();
  ASSERT_EQ(tasks, kFibers * (kYields + 1));
}

TEST(course, MutexWork) {
  exe::sched::RunLoop loop;
  std::vector<size_t> ans;
  exe::fibers::Mutex mutex;
  exe::fibers::Go(loop, [&] {  // ++task
    std::lock_guard lock(mutex);
    ans.push_back(1);
    exe::fibers::Yield();  // ++task
  });
  exe::fibers::Go(loop, [&] {     // ++task
    std::lock_guard lock(mutex);  // ++task
    ans.push_back(2);
    exe::fibers::Yield();  // ++task
  });
  exe::fibers::Go(loop, [&] {     // ++task
    std::lock_guard lock(mutex);  // ++task
    ans.push_back(3);
    exe::fibers::Yield();  // ++task
  });
  EXPECT_EQ(loop.Run(), 8);
  for (size_t i = 1; i < 4; ++i) {
    EXPECT_EQ(ans[i - 1], i);
  }
}

TEST(Mutex, Lock) {
  exe::sched::RunLoop scheduler;
  exe::fibers::Mutex mutex;
  size_t cs = 0;
  exe::fibers::Go(scheduler, [&] {
    mutex.Lock();
    ++cs;
    mutex.Unlock();
    mutex.Lock();
    ++cs;
    mutex.Unlock();
  });
  scheduler.Run();
  ASSERT_EQ(cs, 2);
}

TEST(Mutex, TryLock) {
  exe::sched::RunLoop scheduler;
  exe::fibers::Go(scheduler, [&] {
    exe::fibers::Mutex mutex;
    {
      ASSERT_TRUE(mutex.TryLock());
      mutex.Unlock();
    }

    {
      mutex.Lock();
      mutex.Unlock();
    }

    ASSERT_TRUE(mutex.TryLock());

    bool join = false;

    exe::fibers::Go([&] {
      ASSERT_FALSE(mutex.TryLock());
      join = true;
    });
    while (!join) {
      exe::fibers::Yield();
    }
    mutex.Unlock();
  });

  scheduler.Run();
}

TEST(Mutex, Lockable) {
  exe::sched::RunLoop scheduler;

  exe::fibers::Go(scheduler, [&] {
    exe::fibers::Mutex mutex;

    {
      std::lock_guard guard{mutex};
    }

    {
      std::unique_lock lock{mutex, std::try_to_lock};
      ASSERT_TRUE(lock.owns_lock());
    }
  });

  scheduler.Run();
}

TEST(Mutex, LockManyTimes) {
  exe::sched::RunLoop scheduler;
  exe::fibers::Mutex mutex;
  size_t cs = 0;
  exe::fibers::Go(scheduler, [&] {
    for (size_t j = 0; j < 11; ++j) {
      std::lock_guard guard(mutex);
      ++cs;
    }
  });

  scheduler.Run();

  ASSERT_EQ(cs, 11);
}

TEST(Mutex, Counter) {
  exe::sched::RunLoop scheduler;

  exe::fibers::Mutex mutex;
  size_t cs = 0;

  static const size_t kFibers = 5;
  static const size_t kSectionsPerFiber = 5;

  for (size_t i = 0; i < kFibers; ++i) {
    exe::fibers::Go(scheduler, [&] {
      for (size_t j = 0; j < kSectionsPerFiber; ++j) {
        std::lock_guard guard(mutex);

        ++cs;
        exe::fibers::Yield();
      }
    });
  }

  scheduler.Run();
  ASSERT_EQ(cs, kFibers * kSectionsPerFiber);
}

TEST(Mutex, SuspendFiber) {
  exe::sched::RunLoop scheduler;

  exe::fibers::Mutex mutex;
  exe::fibers::Event unlock;

  exe::fibers::Go(scheduler, [&] {
    mutex.Lock();
    unlock.Wait();
    mutex.Unlock();
  });

  bool cs = false;

  exe::fibers::Go(scheduler, [&] {
    mutex.Lock();
    cs = true;
    mutex.Unlock();
  });

  {
    size_t tasks = scheduler.Run();
    ASSERT_LE(tasks, 17);
    ASSERT_FALSE(cs);
  }

  exe::fibers::Go(scheduler, [&] {
    unlock.Fire();
  });

  scheduler.Run();

  ASSERT_TRUE(cs);
}

TEST(Event, OneWaiter) {
  exe::sched::RunLoop scheduler;

  static const std::string kMessage = "Hello";

  exe::fibers::Event event;
  std::string data;
  bool ok = false;

  exe::fibers::Go(scheduler, [&] {
    event.Wait();
    ASSERT_EQ(data, kMessage);
    ok = true;
  });

  exe::fibers::Go(scheduler, [&] {
    data = kMessage;
    event.Fire();
  });

  scheduler.Run();

  ASSERT_TRUE(ok);
}

TEST(Event, MultipleWaiters) {
  exe::sched::RunLoop scheduler;

  exe::fibers::Event event;
  int work = 0;
  size_t waiters = 0;

  static const size_t kWaiters = 7;

  for (size_t i = 0; i < kWaiters; ++i) {
    exe::fibers::Go(scheduler, [&] {
      event.Wait();
      ASSERT_EQ(work, 1);
      ++waiters;
    });
  }

  exe::fibers::Go(scheduler, [&] {
    ++work;
    event.Fire();
  });

  scheduler.Run();

  ASSERT_EQ(waiters, kWaiters);
}

TEST(Event, SuspendFiber) {
  exe::sched::RunLoop scheduler;

  exe::fibers::Event event;
  bool ok = false;

  exe::fibers::Go(scheduler, [&] {
    event.Wait();
    ok = true;
  });

  {
    size_t tasks = scheduler.Run();
    ASSERT_LE(tasks, 7);
  }

  exe::fibers::Go(scheduler, [&] {
    event.Fire();
  });

  scheduler.Run();

  ASSERT_TRUE(ok);
}

TEST(WaitGroup, OneWaiter) {
  exe::sched::RunLoop scheduler;

  exe::fibers::WaitGroup wg;
  size_t work = 0;
  bool ok = false;

  const size_t kWorkers = 3;

  for (size_t i = 0; i < kWorkers; ++i) {
    wg.Add(1);
    exe::fibers::Go(scheduler, [&] {
      ++work;
      wg.Done();
    });
  }

  exe::fibers::Go(scheduler, [&] {
    wg.Wait();
    ASSERT_EQ(work, kWorkers);
    ok = true;
  });

  scheduler.Run();

  ASSERT_TRUE(ok);
}

TEST(WaitGroup, MultipleWaiters) {
  exe::sched::RunLoop scheduler;

  exe::fibers::WaitGroup wg;

  size_t work = 0;
  size_t acks = 0;

  const size_t kWorkers = 3;
  const size_t kWaiters = 4;

  for (size_t i = 0; i < kWorkers; ++i) {
    wg.Add(1);
    exe::fibers::Go(scheduler, [&] {
      ++work;
      wg.Done();
    });
  }

  for (size_t i = 0; i < kWaiters; ++i) {
    exe::fibers::Go(scheduler, [&] {
      wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ++acks;
    });
  }

  scheduler.Run();

  ASSERT_EQ(acks, kWaiters);
}

TEST(WaitGroup, SuspendFiber) {
  exe::sched::RunLoop scheduler;

  exe::fibers::WaitGroup wg;
  size_t work = 0;
  bool ok = false;

  const size_t kWorkers = 3;

  wg.Add(kWorkers);

  exe::fibers::Go(scheduler, [&] {
    wg.Wait();
    ASSERT_EQ(work, kWorkers);
    ok = true;
  });

  {
    size_t tasks = scheduler.Run();
    ASSERT_LE(tasks, 7);
  }

  for (size_t i = 0; i < kWorkers; ++i) {
    exe::fibers::Go(scheduler, [&] {
      ++work;
      wg.Done();
    });
  }

  scheduler.Run();

  ASSERT_TRUE(ok);
}

TEST(WaitGroup, Cyclic) {
  exe::sched::RunLoop scheduler;

  exe::fibers::WaitGroup wg;

  const size_t kIters = 3;

  for (size_t k = 0; k < kIters; ++k) {
    const size_t kWork = 5;

    size_t work = 0;

    for (size_t i = 0; i < kWork; ++i) {
      wg.Add(1);
      exe::fibers::Go(scheduler, [&] {
        ++work;
        wg.Done();
      });

      exe::fibers::Go(scheduler, [&] {
        wg.Wait();
        ASSERT_EQ(work, kWork);
      });
    }

    scheduler.Run();
  }
}

// CheckThreadpool
TEST(TP, Go) {
  sched::ThreadPool pool{3};
  pool.Start();

  sync::WaitGroup wg;

  wg.Add(1);
  fibers::Go(pool, [&]() {
    ASSERT_TRUE(&pool == sched::ThreadPool::Current());
    wg.Done();
  });

  wg.Wait();

  pool.Stop();
}

TEST(TP, GoGroup) {
  sched::ThreadPool pool{4};
  pool.Start();

  sync::WaitGroup wg;

  const size_t kFibers = 7;

  for (size_t i = 0; i < kFibers; ++i) {
    wg.Add(1);
    fibers::Go(pool, [&] {
      ASSERT_TRUE(&pool == sched::ThreadPool::Current());
      wg.Done();
    });
  }

  wg.Wait();

  pool.Stop();
}

TEST(TP, GoChild) {
  sched::ThreadPool pool{3};
  pool.Start();

  sync::WaitGroup wg;

  wg.Add(1);

  fibers::Go(pool, [&] {
    ASSERT_TRUE(sched::ThreadPool::Current() == &pool);

    wg.Add(1);
    fibers::Go([&] {
      ASSERT_TRUE(sched::ThreadPool::Current() == &pool);
      wg.Done();
    });

    wg.Done();
  });

  wg.Wait();

  pool.Stop();
}

TEST(TP, Yield) {
  sched::ThreadPool pool{1};
  pool.Start();

  sync::WaitGroup wg;

  wg.Add(1);
  fibers::Go(pool, [&] {
    fibers::Yield();

    ASSERT_TRUE(sched::ThreadPool::Current() == &pool);

    wg.Done();
  });

  wg.Wait();

  pool.Stop();
}

TEST(TP, YieldChild) {
  sched::ThreadPool loop{1};
  loop.Start();

  sync::WaitGroup wg;

  wg.Add(1);
  fibers::Go(loop, [&wg] {
    bool child = false;

    fibers::Go([&] {
      child = true;
    });

    while (!child) {
      fibers::Yield();
    }

    wg.Done();
  });

  wg.Wait();

  loop.Stop();
}

TEST(TP, ForYield) {
  sched::ThreadPool loop{1};
  loop.Start();

  const size_t kYields = 128;

  size_t yields = 0;

  sync::WaitGroup wg;

  wg.Add(1);

  fibers::Go(loop, [&] {
    for (size_t i = 0; i < kYields; ++i) {
      fibers::Yield();
      ++yields;
    }

    wg.Done();
  });

  wg.Wait();

  ASSERT_EQ(yields, kYields);

  loop.Stop();
}

TEST(TP, PingPong) {
  sched::ThreadPool loop{1};
  loop.Start();

  bool start = false;
  int turn = 0;

  sync::WaitGroup wg;

  wg.Add(2);

  const size_t kRounds = 3;

  fibers::Go(loop, [&] {
    while (!start) {
      fibers::Yield();
    }

    for (size_t i = 0; i < kRounds; ++i) {
      ASSERT_TRUE(turn == 0);
      turn ^= 1;

      fibers::Yield();
    }

    wg.Done();
  });

  fibers::Go(loop, [&] {
    {
      start = true;
      fibers::Yield();
    }

    for (size_t j = 0; j < kRounds; ++j) {
      ASSERT_TRUE(turn == 1);
      turn ^= 1;

      fibers::Yield();
    }

    wg.Done();
  });

  wg.Wait();

  loop.Stop();
}

TEST(TP, YieldGroup) {
  sched::ThreadPool pool{4};
  pool.Start();

  const size_t kFibers = 5;
  const size_t kYields = 7;

  sync::WaitGroup wg;

  for (size_t i = 0; i < kFibers; ++i) {
    wg.Add(1);

    fibers::Go(pool, [&] {
      for (size_t j = 0; j < kYields; ++j) {
        fibers::Yield();
      }
      wg.Done();
    });
  }

  wg.Wait();

  pool.Stop();
}

TEST(TP, TwoPools) {
  sched::ThreadPool pool1{3};
  pool1.Start();

  sched::ThreadPool pool2{3};
  pool2.Start();

  sync::WaitGroup wg;

  wg.Add(2);

  fibers::Go(pool1, [&] {
    for (size_t i = 0; i < 2; ++i) {
      fibers::Yield();
      ASSERT_TRUE(sched::ThreadPool::Current() == &pool1);
    }
    wg.Done();
  });

  fibers::Go(pool2, [&] {
    for (size_t j = 0; j < 3; ++j) {
      fibers::Yield();
      ASSERT_TRUE(sched::ThreadPool::Current() == &pool2);
    }
    wg.Done();
  });

  wg.Wait();

  pool1.Stop();
  pool2.Stop();
}

TEST(Strand, Lock) {
  sched::RunLoop scheduler;

  fibers::Strand mutex;
  size_t cs = 0;

  fibers::Go(scheduler, [&] {
    mutex.Combine([&] {
      ++cs;
    });

    ASSERT_EQ(cs, 1);

    mutex.Combine([&] {
      ++cs;
    });

    ASSERT_EQ(cs, 2);
  });

  scheduler.Run();

  ASSERT_EQ(cs, 2);
}

TEST(Strand, LockManyTimes) {
  sched::RunLoop scheduler;

  fibers::Strand mutex;
  size_t cs = 0;

  fibers::Go(scheduler, [&] {
    for (size_t j = 0; j < 11; ++j) {
      mutex.Combine([&] {
        ++cs;
      });
    }
  });

  scheduler.Run();

  ASSERT_EQ(cs, 11);
}

TEST(Strand, Counter) {
  sched::RunLoop scheduler;

  fibers::Strand mutex;
  size_t cs = 0;

  static const size_t kFibers = 5;
  static const size_t kSectionsPerFiber = 5;

  for (size_t i = 0; i < kFibers; ++i) {
    fibers::Go(scheduler, [&] {
      for (size_t j = 0; j < kSectionsPerFiber; ++j) {
        mutex.Combine([&] {
          ++cs;
        });
        fibers::Yield();
      }
    });
  }

  scheduler.Run();

  ASSERT_EQ(cs, kFibers * kSectionsPerFiber);
}

TEST(Channel, JustWorks) {
  sched::RunLoop loop;

  bool done = false;

  fibers::Go(loop, [&done] {
    fibers::BufferedChannel<int> chan{7};

    chan.Send(1);
    chan.Send(2);
    chan.Send(3);

    ASSERT_EQ(chan.Recv(), 1);
    ASSERT_EQ(chan.Recv(), 2);
    ASSERT_EQ(chan.Recv(), 3);

    done = true;
  });

  size_t tasks = loop.Run();

  ASSERT_EQ(tasks, 1);
  ASSERT_TRUE(done);
}

struct MoveOnly {
  MoveOnly() = default;

  MoveOnly(const MoveOnly&) = delete;
  MoveOnly& operator=(const MoveOnly&) = delete;
  MoveOnly(MoveOnly&&) = default;
  MoveOnly& operator=(MoveOnly&&) = default;
};

TEST(Channel, MoveOnlyItem) {
  sched::RunLoop loop;

  fibers::Go(loop, [] {
    fibers::BufferedChannel<MoveOnly> chan{3};

    chan.Send({});
    chan.Send({});

    chan.Recv();
    chan.Recv();
  });

  loop.Run();
}

struct NonDefaultConstructible {
  NonDefaultConstructible(int) {
  }
};

TEST(Channel, NonDefaultConstructibleItem) {
  sched::RunLoop loop;

  fibers::Go(loop, [] {
    fibers::BufferedChannel<NonDefaultConstructible> chan{1};

    chan.Send({42});

    chan.Recv();
  });

  loop.Run();
}

TEST(Channel, SuspendReceiver) {
  sched::RunLoop loop;

  fibers::BufferedChannel<int> chan{3};

  bool done = false;

  // Receiver
  fibers::Go(loop, [&done, chan]() mutable {
    int value = chan.Recv();  // <-- Suspended
    ASSERT_EQ(value, 17);
    done = true;
  });

  {
    size_t tasks = loop.Run();
    ASSERT_TRUE(tasks < 3);
    ASSERT_FALSE(done);
  }

  // Sender
  fibers::Go(loop, [chan]() mutable {
    chan.Send(17);  // <-- Resume suspended receiver
  });

  loop.Run();

  ASSERT_TRUE(done);
}

TEST(Channel, SuspendSender) {
  sched::RunLoop loop;

  fibers::BufferedChannel<int> chan{2};

  int sent = 0;

  // Sender
  fibers::Go(loop, [&sent, chan]() mutable {
    for (int v = 0; v < 3; ++v) {
      chan.Send(v);
      ++sent;
    }
  });

  {
    size_t tasks = loop.Run();
    ASSERT_TRUE(tasks < 5);
    ASSERT_EQ(sent, 2);
  }

  bool done = false;

  // Receiver
  fibers::Go(loop, [&done, chan]() mutable {
    {
      int v = chan.Recv();  // <-- Resume suspended sender
      ASSERT_EQ(v, 0);
    }

    fibers::Yield();

    ASSERT_EQ(chan.Recv(), 1);
    ASSERT_EQ(chan.Recv(), 2);

    done = true;
  });

  // Recv + Complete sender
  loop.RunAtMost(2);

  ASSERT_EQ(sent, 3);

  // Complete receiver
  loop.Run();

  ASSERT_TRUE(done);
}

TEST(Channel, Fifo) {
  sched::RunLoop loop;

  fibers::BufferedChannel<int> chan{7};

  const int kMessages = 128;

  fibers::Go(loop, [chan]() mutable {
    for (int i = 0; i < kMessages; ++i) {
      chan.Send(i);

      if (i % 3 == 0) {
        fibers::Yield();
      }
    }
  });

  bool done = false;

  fibers::Go(loop, [&done, chan]() mutable {
    for (int j = 0; j < kMessages; ++j) {
      ASSERT_EQ(chan.Recv(), j);

      if (j % 2 == 0) {
        fibers::Yield();
      }
    }
    done = true;
  });

  {
    size_t tasks = loop.Run();
    ASSERT_TRUE(tasks < 512);
  }

  ASSERT_TRUE(done);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
