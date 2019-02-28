////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef VORONOIRENERATOR_HPP
#define VORONOIRENERATOR_HPP

#include "Beachline.hpp"
#include "Circle.hpp"
#include "Site.hpp"
#include "Vector2f.hpp"

#include <map>
#include <queue>
#include <vector>

typedef std::pair<Vector2f, Vector2f> Edge;

class VoronoiGenerator {
 public:
  VoronoiGenerator();

  void parse(std::vector<Site> const& sites);

  double sweepLine() const;

  double maxY() const;
  double minY() const;

  std::vector<Site> const&                           getSites() const;
  std::vector<Edge> const&                           getEdges() const;
  std::vector<Edge> const&                           getTriangulation() const;
  std::map<unsigned short, std::vector<Site>> const& getNeighbors() const;

  void addTriangulationEdge(Site const& site1, Site const& site2);

 private:
  void process(Site const& event);
  void process(Circle* event);
  void addCircleEvent(Arc* arc);
  void finishEdges();

  Beachline beachline_;
  double    sweepLine_;
  double    maxY_, minY_;

  std::priority_queue<Site, std::vector<Site>, SitePosComp>        siteEvents_;
  std::priority_queue<Circle*, std::vector<Circle*>, CirclePtrCmp> circleEvents_;

  std::vector<Site>                           sites_;
  std::vector<Edge>                           voronoiEdges_;
  std::vector<Edge>                           triangulationEdges_;
  std::map<unsigned short, std::vector<Site>> neighbors_;
};

#endif // VORONOIRENERATOR_HPP
