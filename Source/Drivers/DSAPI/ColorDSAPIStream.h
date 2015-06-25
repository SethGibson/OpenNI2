#ifndef _COLOR_DSAPI_STREAM_H_
#define _COLOR_DSAPI_STREAM_H_

#include "BaseDSAPIStream.h"

namespace dsapi_device
{
  class ColorDSAPIStream : public BaseDSAPIStream
  {
    public:
      ColorDSAPIStream(DSAPIStreamImpl* pStreamImpl);
      virtual void frameReady(void* data, int width, int height, double timestamp);
  };
}

#endif