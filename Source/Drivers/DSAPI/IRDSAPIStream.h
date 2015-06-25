#ifndef _IR_DSAPI_STREAM_H_
#define _IR_DSAPI_STREAM_H_

#include "BaseDSAPIStream.h"

struct IInfraredFrameReader;

namespace dsapi_device
{
  class IRDSAPIStream : public BaseDSAPIStream
  {
    public:
      IRDSAPIStream(DSAPIStreamImpl* pStreamImpl);
      virtual void frameReady(void* data, int width, int height, double timestamp);
  };
}
#endif