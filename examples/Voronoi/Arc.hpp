////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef ARC_HPP
#define ARC_HPP

#include "Circle.hpp"
#include "Site.hpp"

class Breakpoint;

struct Arc {
  Arc(Site const& site);

  void invalidateEvent();

  Site        site;
  Breakpoint *leftBreak, *rightBreak;

  Circle* event;
};

#endif // ARC_HPP
