////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Beachline.hpp"

#include "Arc.hpp"
#include "VoronoiGenerator.hpp"

Beachline::Beachline(VoronoiGenerator* parent)
    : breakPoints_()
    , parent_(parent)
    , root_(NULL) {
}

Arc* Beachline::insertArcFor(Site const& site) {
  // if site creates the very first Arc of the Beachline
  if (root_ == NULL) {
    root_ = new Arc(site);
    return root_;
  }

  Arc* newArc = new Arc(site);

  Arc* brokenArcLeft = breakPoints_.empty() ? root_ : breakPoints_.getArcAt(site.x);
  brokenArcLeft->invalidateEvent();

  // site inserted at exactly the same height as brokenArcLeft
  if (site.y == brokenArcLeft->site.y) {
    if (site.x < brokenArcLeft->site.x) {
      newArc->rightBreak = new Breakpoint(newArc, brokenArcLeft, parent_);
      parent_->addTriangulationEdge(brokenArcLeft->site, newArc->site);
      brokenArcLeft->leftBreak = newArc->rightBreak;
      breakPoints_.insert(newArc->rightBreak);
    }
    // new one is right of brokenArcLeft
    else {
      newArc->leftBreak = new Breakpoint(brokenArcLeft, newArc, parent_);
      parent_->addTriangulationEdge(brokenArcLeft->site, newArc->site);
      brokenArcLeft->rightBreak = newArc->leftBreak;
      breakPoints_.insert(newArc->leftBreak);
    }
  } else {
    Arc* brokenArcRight = new Arc(brokenArcLeft->site);

    newArc->leftBreak  = new Breakpoint(brokenArcLeft, newArc, parent_);
    newArc->rightBreak = new Breakpoint(newArc, brokenArcRight, parent_);

    parent_->addTriangulationEdge(brokenArcLeft->site, newArc->site);

    brokenArcRight->rightBreak = brokenArcLeft->rightBreak;
    if (brokenArcRight->rightBreak) {
      brokenArcRight->rightBreak->rightArc->leftBreak->leftArc = brokenArcRight;
    }
    brokenArcRight->leftBreak = newArc->rightBreak;
    brokenArcLeft->rightBreak = newArc->leftBreak;

    breakPoints_.insert(newArc->leftBreak);
    breakPoints_.insert(newArc->rightBreak);
  }

  return newArc;
}

void Beachline::removeArc(Arc* arc) {
  Arc* leftArc  = arc->leftBreak ? arc->leftBreak->leftArc : NULL;
  Arc* rightArc = arc->rightBreak ? arc->rightBreak->rightArc : NULL;

  arc->invalidateEvent();
  if (leftArc)
    leftArc->invalidateEvent();
  if (rightArc)
    rightArc->invalidateEvent();

  if (leftArc && rightArc) {
    Breakpoint* merged = new Breakpoint(leftArc, rightArc, parent_);

    parent_->addTriangulationEdge(leftArc->site, rightArc->site);

    leftArc->rightBreak = merged;
    rightArc->leftBreak = merged;

    breakPoints_.remove(arc->rightBreak);
    breakPoints_.remove(arc->leftBreak);

    breakPoints_.insert(merged);

    delete arc->leftBreak;
    delete arc->rightBreak;
  } else if (leftArc) {
    breakPoints_.remove(arc->leftBreak);
    leftArc->rightBreak = NULL;
    delete arc->leftBreak;
  } else if (rightArc) {
    breakPoints_.remove(arc->rightBreak);
    rightArc->leftBreak = NULL;
    delete arc->rightBreak;
  }

  delete arc;
}

void Beachline::finish(std::vector<Edge>& edges) {
  breakPoints_.finishAll(edges);
}
