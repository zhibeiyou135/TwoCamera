//
// Created by pe on 2020/7/17.
//

#ifndef TWOCAMERA_DVSDATASEGMENT_H
#define TWOCAMERA_DVSDATASEGMENT_H
#include <cstdint>

class DvsDataSegment {
public:
  uint64_t logTimeStamp;
  uint32_t dataLen;
  uint8_t *data;
};

#endif // TWOCAMERA_DVSDATASEGMENT_H
