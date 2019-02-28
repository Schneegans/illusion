////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Site.hpp"

Site::Site(double x_in, double y_in, unsigned short a)
    : x(x_in)
    , y(y_in)
    , addr(a) {
}

bool operator<(Site const& lhs, Site const& rhs) {
  return lhs.addr < rhs.addr;
}

bool operator==(Site const& lhs, Site const& rhs) {
  return lhs.addr == rhs.addr;
}
