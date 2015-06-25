#ifndef _BASE_DSAPI_STREAM_H_
#define _BASE_DSAPI_STREAM_H_

#include "Driver\OniDriverAPI.h"

namespace dsapi_device
{
  class DSAPIStreamImpl;

  class BaseDSAPIStream : public oni::driver::StreamBase
  {
    public:
      BaseDSAPIStream(DSAPIStreamImpl* pStreamImpl);
      virtual ~BaseDSAPIStream();

      virtual OniStatus start();
      virtual void stop();

      virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
      virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
      virtual OniBool isPropertySupported(int propertyId);

      virtual OniStatus SetVideoMode(OniVideoMode* pVideoMode);
      virtual OniStatus GetVideoMode(OniVideoMode* pVideoMode);

      virtual OniStatus SetCropping(OniCropping* cropping);
      virtual OniStatus GetCropping(OniCropping* cropping);

      bool isRunning() { return m_running; }

      virtual void frameReady(void* data, int width, int height, double timestamp) = 0;

    protected:
      DSAPIStreamImpl *m_pStreamImpl;
      OniVideoMode m_videoMode;
      OniCropping m_cropping;
      bool m_running;
      int m_frameIdx;
  };
}
#endif
