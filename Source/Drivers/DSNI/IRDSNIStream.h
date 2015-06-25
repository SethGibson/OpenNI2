#ifndef _IR_DSAPI_STREAM_H_
#define _IR_DSAPI_STREAM_H_

#include "BaseDSNIStream.h"

struct IInfraredFrameReader;

namespace dsni_device
{
  class IRDSNIStream : public BaseDSNIStream
  {
    public:
      IRDSNIStream(DSNIStreamImpl* pStreamImpl);
      virtual void frameReady(void* data, int width, int height, double timestamp);
  };
}
#endif