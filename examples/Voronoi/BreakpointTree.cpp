////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "BreakpointTree.hpp"
#include "Arc.hpp"
#include "Breakpoint.hpp"

#include <cmath>
#include <limits>

BreakpointTree::BreakpointTree()
    : root_(0) {
}

BreakpointTree::~BreakpointTree() {
  clear(root_);
}

void BreakpointTree::insert(Breakpoint* point) {
  if (empty())
    root_ = point;
  else
    insert(point, root_);
}

void BreakpointTree::remove(Breakpoint* point) {
  if (point->parent == NULL) {

    if (point->leftChild && point->rightChild) {
      root_         = point->rightChild;
      root_->parent = NULL;
      attachLeftOf(point->leftChild, root_);
    } else if (point->leftChild) {
      root_         = point->leftChild;
      root_->parent = NULL;
    } else if (point->rightChild) {
      root_         = point->rightChild;
      root_->parent = NULL;
    } else {
      root_ = NULL;
    }
  } else {

    const bool isLeftChild(point == point->parent->leftChild);

    if (point->leftChild && point->rightChild) {
      if (isLeftChild) {
        point->parent->leftChild = point->leftChild;
        point->leftChild->parent = point->parent;
        attachRightOf(point->rightChild, point->parent->leftChild);
      } else {
        point->parent->rightChild = point->rightChild;
        point->rightChild->parent = point->parent;
        attachLeftOf(point->leftChild, point->parent->rightChild);
      }
    } else if (point->leftChild) {
      if (isLeftChild)
        point->parent->leftChild = point->leftChild;
      else
        point->parent->rightChild = point->leftChild;
      point->leftChild->parent = point->parent;
    } else if (point->rightChild) {
      if (isLeftChild)
        point->parent->leftChild = point->rightChild;
      else
        point->parent->rightChild = point->rightChild;
      point->rightChild->parent = point->parent;
    } else {
      if (isLeftChild)
        point->parent->leftChild = NULL;
      else
        point->parent->rightChild = NULL;
    }
  }
}

Arc* BreakpointTree::getArcAt(double x) const {

  Breakpoint* nearest = getNearestNode(x, root_);

  if (x < nearest->position().x) {
    return nearest->leftArc;
  } else {
    return nearest->rightArc;
  }
}

bool BreakpointTree::empty() const {
  return !root_;
}

void BreakpointTree::insert(Breakpoint* newNode, Breakpoint* atNode) {
  double newX = newNode->position().x;
  double atX  = atNode->position().x;
  if (newX < atX || (newX == atX && newNode->rightArc == atNode->leftArc)) {
    if (atNode->leftChild)
      insert(newNode, atNode->leftChild);
    else {
      atNode->leftChild = newNode;
      newNode->parent   = atNode;
    }
  } else {
    if (atNode->rightChild)
      insert(newNode, atNode->rightChild);
    else {
      atNode->rightChild = newNode;
      newNode->parent    = atNode;
    }
  }
}

Breakpoint* BreakpointTree::getNearestNode(double x, Breakpoint* current) const {
  if (!current)
    return NULL;

  Breakpoint* nearestChild = (x < current->position().x) ? getNearestNode(x, current->leftChild)
                                                         : getNearestNode(x, current->rightChild);
  Breakpoint* nearest = current;

  if (nearestChild &&
      (std::fabs(x - nearestChild->position().x) < std::fabs(x - nearest->position().x)))
    nearest = nearestChild;

  return nearest;
}

void BreakpointTree::finishAll(std::vector<Edge>& edges) {
  finishAll(edges, root_);
}

void BreakpointTree::finishAll(std::vector<Edge>& edges, Breakpoint* atNode) {
  if (atNode) {
    edges.push_back(atNode->finishEdge(atNode->position()));
    finishAll(edges, atNode->leftChild);
    finishAll(edges, atNode->rightChild);
  }
}

void BreakpointTree::clear(Breakpoint* atNode) {
  if (atNode) {
    clear(atNode->leftChild);
    clear(atNode->rightChild);

    if (atNode->leftArc) {
      if (atNode->leftArc->leftBreak)
        atNode->leftArc->leftBreak->rightArc = NULL;
      delete atNode->leftArc;
    }

    if (atNode->rightArc) {
      if (atNode->rightArc->rightBreak)
        atNode->rightArc->rightBreak->leftArc = NULL;
      delete atNode->rightArc;
    }

    delete atNode;
  }
}

void BreakpointTree::attachRightOf(Breakpoint* newNode, Breakpoint* atNode) {
  if (atNode->rightChild)
    attachRightOf(newNode, atNode->rightChild);
  else {
    atNode->rightChild = newNode;
    newNode->parent    = atNode;
  }
}

void BreakpointTree::attachLeftOf(Breakpoint* newNode, Breakpoint* atNode) {
  if (atNode->leftChild)
    attachLeftOf(newNode, atNode->leftChild);
  else {
    atNode->leftChild = newNode;
    newNode->parent   = atNode;
  }
}
