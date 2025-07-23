//
// Created by pe on 2021/5/6.
//

#ifndef TWOCAMERA_EVENTSTRUCT_H
#define TWOCAMERA_EVENTSTRUCT_H

#include <cstdint>

struct Event {
  int x;
  int y;
  int g;
  uint64_t t;
};
#endif // TWOCAMERA_EVENTSTRUCT_H
