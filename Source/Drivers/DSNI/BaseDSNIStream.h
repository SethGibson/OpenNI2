#ifndef _BASE_DSNI_STREAM_H_
#define _BASE_DSNI_STREAM_H_

#include "Driver\OniDriverAPI.h"

namespace dsni_device
{
  class DSNIStreamImpl;

  class BaseDSNIStream : public oni::driver::StreamBase
  {
    public:
      BaseDSNIStream(DSNIStreamImpl* pStreamImpl);
      virtual ~BaseDSNIStream();

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
      DSNIStreamImpl *m_pStreamImpl;
      OniVideoMode m_videoMode;
      OniCropping m_cropping;
      bool m_running;
      int m_frameIdx;
  };
}
#endif
