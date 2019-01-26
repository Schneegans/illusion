////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_THREAD_POOL_HPP
#define ILLUSION_CORE_THREAD_POOL_HPP

#include <future>
#include <queue>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The ThreadPool can be used to execute tasks in parallel. New work can be pushed into a queue   //
// and it will be processed by a set of threads asynchronously.                                   //
////////////////////////////////////////////////////////////////////////////////////////////////////

class ThreadPool {
 public:
  // Constructs a new ThreadPool with a given number of worker-threads. If the argument is zero,
  // std::thread::hardware_concurrency() will be used. If, for some reason, this equals to zero as
  // well, only one thread will be launched. You can use setThreadCount() to adjust the number
  // later and getThreadCount() to check how many threads have been launched.
  ThreadPool(uint32_t threadCount = 0);

  // The destructor will wait until all currently running tasks are done, however tasks which are
  // still in the queue will be discarded. Use waitIdle() before the destructor to make sure that
  // all tasks have been processed.
  virtual ~ThreadPool();

  // Changes the number of worker threads. If the argument is zero,
  // std::thread::hardware_concurrency() will be used. If, for some reason, this equals to zero as
  // well, only one thread will be launched. This method will block until all tasks which are
  // currently processed are finished. Any tasks which are queued and not yet in processing, will be
  // executed afterwards.
  void setThreadCount(uint32_t count);

  // You can use getThreadCount() to check how many threads have been launched.
  uint32_t getThreadCount() const;

  // This will do a busy wait until all pending tasks have been executed.
  void waitIdle();

  // Returns the number of tasks which are currently processed.
  uint32_t getRunningTasks() const;

  // Returns the number of tasks which are queued and not yet running.
  uint32_t getPendingTasks() const;

  // This is the main method to queue new tasks. It will return a std::future object which can be
  // queried for the result of the queued task.
  template <typename F>
  std::future<typename std::result_of<F()>::type> enqueue(F&& f) {

    // Wrap the given function in a std::packaged_task so that we can return a std::future for the
    // result of the function.
    auto task = std::make_shared<std::packaged_task<typename std::result_of<F()>::type()>>(
        std::forward<F>(f));

    // We will return the result of the task as a std::future.
    auto futureResult = task->get_future();

    // Add a lambda to our task list which executes the new task.
    {
      std::unique_lock<std::mutex> lock(mMutex);
      mTasks.emplace([task]() { (*task)(); });
    }

    // Notify any waiting thread that there is new work to be done.
    mCondition.notify_one();

    // Return our future result.
    return futureResult;
  }

 private:
  void stop();
  void restart(uint32_t threadCount);

  std::vector<std::thread>          mThreads;
  std::queue<std::function<void()>> mTasks;
  mutable std::mutex                mMutex;
  std::condition_variable           mCondition;
  std::atomic_uint                  mRunningTasks = 0;
  bool                              mRunning      = false;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_THREAD_POOL_HPP
