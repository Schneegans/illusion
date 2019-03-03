////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include <Illusion/Core/ThreadPool.hpp>

#include <doctest.h>
#include <iostream>
#include <random>

namespace Illusion::Core {

TEST_CASE("Illusion::Core::ThreadPool") {
  ThreadPool pool;
  CHECK(pool.getThreadCount() != 0);

  // First try setting the thread count.
  pool.setThreadCount(7);
  CHECK(pool.getThreadCount() == 7);

  // Set once more, this won't start new threads.
  pool.setThreadCount(7);
  CHECK(pool.getThreadCount() == 7);

  // Once more, this should start new threads.
  pool.setThreadCount(10);
  CHECK(pool.getThreadCount() == 10);

  // There shouldn't be any tasks running or pending.
  CHECK(pool.getRunningTasks() == 0);
  CHECK(pool.getPendingTasks() == 0);

  // Now do some actual work.
  std::atomic_uint counter = 0;
  for (uint32_t i(0); i < 1000; ++i) {
    pool.enqueue([&]() {
      std::default_random_engine              generator;
      std::uniform_int_distribution<uint32_t> time(0, 10);
      std::this_thread::sleep_for(std::chrono::microseconds(time(generator)));
      ++counter;
    });
  }

  pool.waitIdle();

  CHECK(counter == 1000u);
}

} // namespace Illusion::Core
