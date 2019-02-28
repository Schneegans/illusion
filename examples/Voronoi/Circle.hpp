////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CIRCLEEVENT_HPP
#define CIRCLEEVENT_HPP

#include "Site.hpp"
#include "Vector2f.hpp"

struct Arc;
struct Site;

struct Circle {
  Circle(Arc* arc, double sweepLine);

  Site     site;
  Vector2f center;
  Arc*     arc;
  bool     isValid;
  Vector2f priority;
};

bool operator<(Circle const& lhs, Circle const& rhs);

struct CirclePtrCmp {
  bool operator()(const Circle* lhs, const Circle* rhs) const {
    return *lhs < *rhs;
  }
};

#endif // CIRCLEEVENT_HPP
