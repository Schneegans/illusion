////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_QUEUE_HPP
#define ILLUSION_CORE_QUEUE_HPP

// ---------------------------------------------------------------------------------------- includes
#include <mutex>
#include <queue>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A thread safe queue.                                                                           //
////////////////////////////////////////////////////////////////////////////////////////////////////

// -------------------------------------------------------------------------------------------------
template <class T>
class Queue {

 public:
  // -------------------------------------------------------------------------------- public methods
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

    if (mQueue.empty()) return false;

    val = mQueue.front();
    mQueue.pop();

    return true;
  }

 private:
  // ------------------------------------------------------------------------------ private members
  std::queue<T>      mQueue;
  mutable std::mutex mMutex;
};

// -------------------------------------------------------------------------------------------------
} // namespace Illusion::Core

#endif // ILLUSION_CORE_QUEUE_HPP
