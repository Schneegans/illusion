////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//   _)  |  |            _)                 This software may be modified and distributed         //
//    |  |  |  |  | (_-<  |   _ \    \      under the terms of the MIT license.                   //
//   _| _| _| \_,_| ___/ _| \___/ _| _|     See the LICENSE file for details.                     //
//                                                                                                //
//  Authors: Simon Schneegans (code@simonschneegans.de)                                           //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_SIGNAL_HPP
#define ILLUSION_CORE_SIGNAL_HPP

#include <functional>
#include <map>
#include <mutex>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// A signal object may call multiple callbacks with the same signature. You can connect functions //
// to the signal which will be called when the emit() method on the signal object is invoked. Any //
// argument passed to emit() will be passed to the given function. Connect and disconnect methods //
// may be called from different threads, but the callbacks will be called from the thread calling //
// emit().                                                                                        //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename... Parameters>
class Signal {

 public:
  Signal() = default;

  // copy creates new signal
  Signal(Signal const& other) {}

  // connects a member function of a given object to this Signal
  template <typename F, typename... Args>
  int connectMember(F&& f, Args&&... a) const {
    std::unique_lock<std::mutex> lock(mMutex);
    mCallbacks.insert(std::make_pair(++mCurrentId, std::bind(f, a...)));
    return mCurrentId;
  }

  // connects a std::function to the signal. The returned value can be used to
  // disconnect the function again
  int connect(std::function<bool(Parameters...)> const& callback) const {
    std::unique_lock<std::mutex> lock(mMutex);
    mCallbacks.insert(std::make_pair(++mCurrentId, callback));
    return mCurrentId;
  }

  // disconnects a previously connected function
  void disconnect(int id) const {
    std::unique_lock<std::mutex> lock(mMutex);
    mCallbacks.erase(id);
  }

  // disconnects all previously connected functions
  void disconnectAll() const {
    std::unique_lock<std::mutex> lock(mMutex);
    mCallbacks.clear();
    mCurrentId = 0;
  }

  // calls all connected functions
  void emit(Parameters... p) {
    std::unique_lock<std::mutex> lock(mMutex);
    auto                         it(mCallbacks.begin());
    while (it != mCallbacks.end()) {
      if (it->second(p...)) {
        ++it;
      } else {
        it = mCallbacks.erase(it);
      }
    }
  }

  // assignment creates new Signal
  Signal& operator=(Signal const& other) { disconnectAll(); }

 private:
  mutable std::map<int, std::function<bool(Parameters...)>> mCallbacks;
  mutable int                                               mCurrentId = 0;

  mutable std::mutex mMutex;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_SIGNAL_HPP
