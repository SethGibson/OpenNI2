#ifndef _COLOR_DSNI_STREAM_H_
#define _COLOR_DSNI_STREAM_H_

#include "BaseDSNIStream.h"

namespace dsni_device
{
  class ColorDSNIStream : public BaseDSNIStream
  {
    public:
      ColorDSNIStream(DSNIStreamImpl* pStreamImpl);
      virtual void frameReady(void* data, int width, int height, double timestamp);
  };
}

#endif