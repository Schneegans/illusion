////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BEACHLINE_HPP
#define BEACHLINE_HPP

#include "Breakpoint.hpp"
#include "BreakpointTree.hpp"
#include "Site.hpp"

class VoronoiGenerator;

class Beachline {
 public:
  Beachline(VoronoiGenerator* parent);

  Arc* insertArcFor(Site const& site);
  void removeArc(Arc* arc);
  void finish(std::vector<Edge>& edges);

 private:
  BreakpointTree    breakPoints_;
  VoronoiGenerator* parent_;
  Arc*              root_;
};

#endif // BEACHLINE_HPP
