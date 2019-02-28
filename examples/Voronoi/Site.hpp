////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef SITE_HPP
#define SITE_HPP

struct Site {
  Site(double x, double y, unsigned short a = 0);

  double x;
  double y;

  unsigned short addr;
};

bool operator<(Site const& lhs, Site const& rhs);

bool operator==(Site const& lhs, Site const& rhs);

struct SitePosComp {
  bool operator()(Site const& lhs, Site const& rhs) const {
    return (lhs.y == rhs.y) ? (lhs.x < rhs.x) : (lhs.y > rhs.y);
  }
};

#endif // SITE_HPP
