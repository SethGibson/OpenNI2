#ifndef _DSNI_DRIVER_H_
#define _DSNI_DRIVER_H_

#include "Driver\OniDriverAPI.h"
#include "XnHash.h"

namespace dsni_device
{
  class DSNIDriver : public oni::driver::DriverBase 
  {
    public:
	    DSNIDriver(OniDriverServices* pDriverServices);
	
	    virtual OniStatus initialize(oni::driver::DeviceConnectedCallback connectedCallback,
                                   oni::driver::DeviceDisconnectedCallback disconnectedCallback,
                                   oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,
                                   void* pCookie);
	    virtual ~DSNIDriver();

	    virtual oni::driver::DeviceBase* deviceOpen(const char* uri, const char* mode);
	    virtual void deviceClose(oni::driver::DeviceBase* pDevice);

	    virtual void shutdown();

	    virtual OniStatus tryDevice(const char* uri);

	    virtual void* enableFrameSync(oni::driver::StreamBase** pStreams, int streamCount);
	    virtual void disableFrameSync(void* frameSyncGroup);

    private:
	    xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*> m_devices;
  };
}
#endif