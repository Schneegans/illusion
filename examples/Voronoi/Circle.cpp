////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Circle.hpp"
#include "Arc.hpp"
#include "Breakpoint.hpp"

#include <cmath>
#include <limits>

Circle::Circle(Arc* a, double sweepLine)
    : site(a->site)
    , center()
    , arc(a)
    , isValid(true)
    , priority() {

  if (!arc->leftBreak || !arc->rightBreak) {
    isValid = false;
    return;
  }

  Site* site3(&arc->leftBreak->leftArc->site);
  Site* site2(&arc->site);
  Site* site1(&arc->rightBreak->rightArc->site);

  if ((site2->x - site1->x) * (site3->y - site1->y) -
          (site3->x - site1->x) * (site2->y - site1->y) >
      0) {
    isValid = false;
    return;
  }

  // Algorithm from O'Rourke 2ed p. 189.
  const double A = site2->x - site1->x;
  const double B = site2->y - site1->y;
  const double C = site3->x - site1->x;
  const double D = site3->y - site1->y;
  const double E = A * (site1->x + site2->x) + B * (site1->y + site2->y);
  const double F = C * (site1->x + site3->x) + D * (site1->y + site3->y);
  const double G = 2 * (A * (site3->y - site2->y) - B * (site3->x - site2->x));

  // Points are co-linear.
  if (std::fabs(G) <= std::numeric_limits<double>::epsilon()) {
    isValid = false;
    return;
  }

  center.x = (D * E - B * F) / G;
  center.y = (A * F - C * E) / G;

  priority = Vector2f(center.x, center.y + (center - Vector2f(site1->x, site1->y)).length());

  if (priority.y < sweepLine) {
    isValid = false;
    return;
  }

  arc->event = this;
}

bool operator<(Circle const& lhs, Circle const& rhs) {
  return (lhs.priority.y == rhs.priority.y) ? (lhs.priority.x < rhs.priority.x)
                                            : (lhs.priority.y > rhs.priority.y);
}
