////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BREAKPOINT_HPP
#define BREAKPOINT_HPP

#include "Vector2f.hpp"

struct Arc;
class VoronoiGenerator;

typedef std::pair<Vector2f, Vector2f> Edge;

class Breakpoint {
 public:
  Breakpoint();
  Breakpoint(Arc* left, Arc* right, VoronoiGenerator* generator);

  Vector2f const& position() const;
  Edge const      finishEdge(Vector2f const& end) const;

  Arc *       leftArc, *rightArc;
  Breakpoint *leftChild, *rightChild, *parent;

 private:
  void updatePosition() const;

  VoronoiGenerator* generator_;

  mutable double   sweepline_;
  mutable Vector2f position_;

  Vector2f start_;
};

#endif // BREAKPOINT_HPP
