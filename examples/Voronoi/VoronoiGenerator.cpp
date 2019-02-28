////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "VoronoiGenerator.hpp"
#include "Arc.hpp"

#include <iomanip>
#include <limits>

VoronoiGenerator::VoronoiGenerator()
    : beachline_(this)
    , sweepLine_(0.0)
    , maxY_(0.0)
    , minY_(0.0)
    , siteEvents_()
    , circleEvents_()
    , voronoiEdges_()
    , triangulationEdges_()
    , neighbors_() {
}

void VoronoiGenerator::parse(std::vector<Site> const& sites) {
  sites_     = sites;
  beachline_ = Beachline(this);
  sweepLine_ = 0.0;
  maxY_      = 0.0;
  minY_      = 0.0;
  voronoiEdges_.clear();
  triangulationEdges_.clear();
  neighbors_.clear();

  if (sites.size() > 1) {
    std::cout << std::setprecision(1) << std::fixed;

    maxY_ = 600.0;
    minY_ = 0.0;

    for (auto site : sites) {
      if (site.y > maxY_)
        maxY_ = site.y;
      if (site.y < minY_)
        minY_ = site.y;
      siteEvents_.push(site);
    }

    while (!circleEvents_.empty() || !siteEvents_.empty()) {
      double nextSite =
          siteEvents_.empty() ? std::numeric_limits<double>::max() : siteEvents_.top().y;
      double nextCircle = circleEvents_.empty() ? std::numeric_limits<double>::max()
                                                : circleEvents_.top()->priority.y;

      if (nextCircle < nextSite) {
        Circle* next = circleEvents_.top();
        circleEvents_.pop();
        sweepLine_ = next->priority.y;
        process(next);
        delete next;
      } else {
        Site next = siteEvents_.top();
        siteEvents_.pop();

        // hackhack...
        if (!siteEvents_.empty() && siteEvents_.top().y == next.y && siteEvents_.top().x == next.x)
          continue;

        sweepLine_ = next.y;
        process(next);
      }
    }

    finishEdges();
  }
}

double VoronoiGenerator::sweepLine() const {
  return sweepLine_;
}

double VoronoiGenerator::maxY() const {
  return maxY_;
}

double VoronoiGenerator::minY() const {
  return minY_;
}

std::vector<Site> const& VoronoiGenerator::getSites() const {
  return sites_;
}

std::vector<Edge> const& VoronoiGenerator::getEdges() const {
  return voronoiEdges_;
}

std::vector<Edge> const& VoronoiGenerator::getTriangulation() const {
  return triangulationEdges_;
}

std::map<unsigned short, std::vector<Site>> const& VoronoiGenerator::getNeighbors() const {
  return neighbors_;
}

void VoronoiGenerator::addTriangulationEdge(Site const& site1, Site const& site2) {
  triangulationEdges_.push_back(
      std::make_pair(Vector2f(site1.x, site1.y), Vector2f(site2.x, site2.y)));
  neighbors_[site1.addr].push_back(site2);
  neighbors_[site2.addr].push_back(site1);
}

void VoronoiGenerator::process(Circle* event) {

  if (event->isValid) {

    Arc* leftArc  = event->arc->leftBreak ? event->arc->leftBreak->leftArc : NULL;
    Arc* rightArc = event->arc->rightBreak ? event->arc->rightBreak->rightArc : NULL;

    if (leftArc)
      voronoiEdges_.push_back(event->arc->leftBreak->finishEdge(event->center));
    if (rightArc)
      voronoiEdges_.push_back(event->arc->rightBreak->finishEdge(event->center));

    beachline_.removeArc(event->arc);

    addCircleEvent(leftArc);
    addCircleEvent(rightArc);
  }
}

void VoronoiGenerator::process(Site const& event) {

  Arc* newArc = beachline_.insertArcFor(event);

  addCircleEvent(newArc->leftBreak ? newArc->leftBreak->leftArc : NULL);
  addCircleEvent(newArc->rightBreak ? newArc->rightBreak->rightArc : NULL);
}

void VoronoiGenerator::addCircleEvent(Arc* arc) {
  if (arc) {
    auto circle = new Circle(arc, sweepLine());
    if (circle->isValid)
      circleEvents_.push(circle);
    else
      delete circle;
  }
}

void VoronoiGenerator::finishEdges() {
  sweepLine_ = 2 * maxY_;
  beachline_.finish(voronoiEdges_);
}
