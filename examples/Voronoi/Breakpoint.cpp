////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Breakpoint.hpp"
#include "Arc.hpp"
#include "VoronoiGenerator.hpp"

#include <cmath>

Breakpoint::Breakpoint()
    : leftArc(NULL)
    , rightArc(NULL)
    , leftChild(NULL)
    , rightChild(NULL)
    , parent(NULL)
    , generator_(NULL)
    , sweepline_(-1.0)
    , position_()
    , start_() {
}

Breakpoint::Breakpoint(Arc* left, Arc* right, VoronoiGenerator* generator)
    : leftArc(left)
    , rightArc(right)
    , leftChild(NULL)
    , rightChild(NULL)
    , parent(NULL)
    , generator_(generator)
    , sweepline_(-1.0)
    , position_()
    , start_(position()) {
}

Vector2f const& Breakpoint::position() const {
  double currentSweepLine = generator_->sweepLine();

  if (sweepline_ == currentSweepLine)
    return position_;

  sweepline_ = currentSweepLine;
  updatePosition();
  return position_;
}

Edge const Breakpoint::finishEdge(Vector2f const& end) const {
  return std::make_pair(start_, end);
}

void Breakpoint::updatePosition() const {

  double pX = leftArc->site.x;
  double pY = leftArc->site.y;
  double rX = rightArc->site.x;
  double rY = rightArc->site.y;

  if (pY == rY)
    position_.x = (pX + rX) * 0.5;
  else if (rY == sweepline_)
    position_.x = rX;
  else if (pY == sweepline_) {
    position_.x = pX;
    pX          = rX;
    pY          = rY;
  } else {
    double leftDiff  = 2 * (pY - sweepline_);
    double rightDiff = 2 * (rY - sweepline_);

    // Use the quadratic formula.
    double a = 1.0 / leftDiff - 1.0 / rightDiff;
    double b = -2.0 * (pX / leftDiff - rX / rightDiff);
    double c = (pX * pX + pY * pY - sweepline_ * sweepline_) / leftDiff -
               (rX * rX + rY * rY - sweepline_ * sweepline_) / rightDiff;
    position_.x = (-b - std::sqrt(b * b - 4.0 * a * c)) / (2.0 * a);
  }

  // Plug back into one of the parabola equations.
  if (pY != sweepline_)
    position_.y = (pY * pY + (pX - position_.x) * (pX - position_.x) - sweepline_ * sweepline_) /
                  (2.0 * pY - 2.0 * sweepline_);
  else
    position_.y = generator_->minY();
}
