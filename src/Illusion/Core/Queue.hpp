////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)               This code may be used and modified under the terms      //
//    |  |  |  |  | (_-<  |   _ \    \    of the MIT license. See the LICENSE file for details.   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|   Copyright (c) 2018-2019 Simon Schneegans                //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_QUEUE_HPP
#define ILLUSION_CORE_QUEUE_HPP

#include <mutex>
#include <queue>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A thread safe queue.                                                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <class T>
class Queue {

 public:
  bool empty() const {
    std::unique_lock<std::mutex> lock(mMutex);
    return mQueue.empty();
  }

  unsigned size() const {
    std::unique_lock<std::mutex> lock(mMutex);
    return mQueue.size();
  }

  void push(T const& val, unsigned count = 1) {
    std::unique_lock<std::mutex> lock(mMutex);
    for (unsigned i(0); i < count; ++i) {
      mQueue.push(val);
    }
  }

  void push(std::vector<T> const& val) {
    std::unique_lock<std::mutex> lock(mMutex);
    for (auto const& t : val) {
      mQueue.push(t);
    }
  }

  bool pop(T& val) {
    std::unique_lock<std::mutex> lock(mMutex);

    if (mQueue.empty())
      return false;

    val = mQueue.front();
    mQueue.pop();

    return true;
  }

 private:
  std::queue<T>      mQueue;
  mutable std::mutex mMutex;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_QUEUE_HPP
