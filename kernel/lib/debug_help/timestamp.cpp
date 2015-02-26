/* ===================================================================
Copyright (c) 2013, Palo Alto Research Center, Inc.
=================================================================== */

#include "timestamp.h"

std::ostream &operator<<(std::ostream &os, const TimeStamp &ts) {
  os << "(Timestamp " << ts.getName() << "), diff (s):" << ts.getDiff();
  return os;
}
