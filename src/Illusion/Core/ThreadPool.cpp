////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ThreadPool.hpp"

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////

ThreadPool::ThreadPool(uint32_t threadCount) {
  restart(threadCount);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadPool::setThreadCount(uint32_t count) {
  restart(count);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ThreadPool::getThreadCount() const {
  return mThreads.size();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

ThreadPool::~ThreadPool() {
  // Wait until all worker threads have ended.
  stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadPool::waitIdle() {
  // Do busy wait.
  while (getRunningTasks() + getPendingTasks() > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ThreadPool::getRunningTasks() const {
  return mRunningTasks;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint32_t ThreadPool::getPendingTasks() const {
  uint32_t result;
  {
    std::unique_lock<std::mutex> lock(mMutex);
    result = mTasks.size();
  }
  return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadPool::stop() {
  // Set mRunning to false and notify all worker-threads that they can quit.
  {
    std::unique_lock<std::mutex> lock(mMutex);
    mRunning = false;
  }
  mCondition.notify_all();

  // Join the threads.
  for (std::thread& t : mThreads) {
    t.join();
  }

  // Clear the thread list. There are no threads running anymore.
  mThreads.clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void ThreadPool::restart(uint32_t threadCount) {
  // Zero means that we should use as many threads as possibly useful.
  if (threadCount == 0) {
    threadCount = std::thread::hardware_concurrency();
  }

  // Fallback if std::thread::hardware_concurrency() is not supported.
  if (threadCount == 0) {
    threadCount = 1;
  }

  // We do not need to restart if there are currently exactly that many threads running.
  if (threadCount == getThreadCount()) {
    return;
  }

  // End all currently running threads (if any).
  if (mRunning) {
    stop();
  }

  // Now we can start some new threads.
  mRunning = true;

  for (size_t i = 0; i < threadCount; ++i) {
    mThreads.emplace_back([this] {
      while (true) {

        // Try to get a new task from the queue.
        std::function<void()> task;
        {
          std::unique_lock<std::mutex> lock(mMutex);

          // Wait until there is a task in the queue or stop() has been called.
          mCondition.wait(lock, [this] { return !mRunning || !mTasks.empty(); });

          // If stop has been called, we can quit this thread.
          if (!mRunning) {
            return;
          }

          // If there is a new task, pop it from the queue.
          task = std::move(mTasks.front());
          mTasks.pop();

          ++mRunningTasks;
        }

        // And execute it!
        task();
        --mRunningTasks;
      }
    });
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace Illusion::Core
