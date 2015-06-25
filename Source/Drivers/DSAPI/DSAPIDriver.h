#ifndef _DSAPI_DRIVER_H_
#define _DSAPI_DRIVER_H_

#include "Driver\OniDriverAPI.h"
#include "XnHash.h"

namespace dsapi_device
{
  class DSAPIDriver : public oni::driver::DriverBase 
  {
    public:
	    DSAPIDriver(OniDriverServices* pDriverServices);
	
	    virtual OniStatus initialize(oni::driver::DeviceConnectedCallback connectedCallback,
                                   oni::driver::DeviceDisconnectedCallback disconnectedCallback,
                                   oni::driver::DeviceStateChangedCallback deviceStateChangedCallback,
                                   void* pCookie);
	    virtual ~DSAPIDriver();

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