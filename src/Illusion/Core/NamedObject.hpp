////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ILLUSION_CORE_NAMED_OBJECT_HPP
#define ILLUSION_CORE_NAMED_OBJECT_HPP

#include <string>

namespace Illusion::Core {

////////////////////////////////////////////////////////////////////////////////////////////////////
// The NamedObject is a tiny base class for all types which carry a human-readable name. This may //
// be very useful for debugging purposes. Usually the name is supposed to be immutable, but in    //
// order to support default-constructed derived classes, there is a default constructor. When     //
// this is used, derived classes should call setName() as soon as possible.                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

class NamedObject {
 public:
  NamedObject(std::string const& name = "Unamed Object");

  std::string const& getName() const;

 protected:
  void setName(std::string const& name);

 private:
  std::string mName;
};

} // namespace Illusion::Core

#endif // ILLUSION_CORE_NAMED_OBJECT_HPP
