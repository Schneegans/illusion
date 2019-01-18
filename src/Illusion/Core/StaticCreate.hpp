////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_STATIC_CREATE_HPP
#define ILLUSION_CORE_STATIC_CREATE_HPP

#include <memory>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// This template class is used by many classes in Illusion as a base class. It adds some  syn-    //
// tactic sugar allowing to call MyClass::create(...) instaed of std::make_shared<MyClass>(...)   //
// which is much more readable.                                                                   //
// All arguments given to create() will be forwarded to the constructor.                          //
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class StaticCreate {
 public:
  // Add syntactic sugar to create a std::shared_ptr for this class
  template <typename... Params>
  static std::shared_ptr<T> create(Params&&... p) {
    return std::make_shared<T>(std::forward<Params>(p)...);
  }
};
} // namespace Illusion::Core

#endif // ILLUSION_CORE_STATIC_CREATE_HPP
