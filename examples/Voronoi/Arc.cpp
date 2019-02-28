////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//    _)  |  |            _)                This code may be used and modified under the terms    //
//     |  |  |  |  | (_-<  |   _ \    \     of the MIT license. See the LICENSE file for details. //
//    _| _| _| \_,_| ___/ _| \___/ _| _|    Copyright (c) 2018-2019 Simon Schneegans              //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Arc.hpp"
#include "Breakpoint.hpp"

Arc::Arc(Site const& s)
    : site(s)
    , leftBreak(NULL)
    , rightBreak(NULL)
    , event(NULL) {
}

void Arc::invalidateEvent() {

  if (event) {
    if (event->isValid) {
      event->isValid = false;
    }

    event = NULL;
  }
}
