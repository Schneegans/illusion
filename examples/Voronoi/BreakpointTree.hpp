////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef BREAKPOINTTREE_HPP
#define BREAKPOINTTREE_HPP

#include "Vector2f.hpp"

#include <vector>

typedef std::pair<Vector2f, Vector2f> Edge;

class Breakpoint;
struct Arc;

class BreakpointTree {
 public:
  BreakpointTree();
  ~BreakpointTree();

  void insert(Breakpoint* point);
  void remove(Breakpoint* point);
  void finishAll(std::vector<Edge>& edges);

  Arc* getArcAt(double x) const;

  bool empty() const;

 private:
  void        insert(Breakpoint* newNode, Breakpoint* atNode);
  Breakpoint* getNearestNode(double x, Breakpoint* current) const;
  void        finishAll(std::vector<Edge>& edges, Breakpoint* atNode);
  void        clear(Breakpoint* atNode);

  void attachRightOf(Breakpoint* newNode, Breakpoint* atNode);
  void attachLeftOf(Breakpoint* newNode, Breakpoint* atNode);

  Breakpoint* root_;
};

#endif // BREAKPOINTTREE_HPP
