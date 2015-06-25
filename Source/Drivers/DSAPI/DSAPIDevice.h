#ifndef _DSAPI_DEVICE_H_
#define _DSAPI_DEVICE_H_

#include "Driver\OniDriverAPI.h"
#include "DSAPIStreamImpl.h"

namespace dsapi_device
{
	class DSAPIDevice : public oni::driver::DeviceBase
	{
	public:
		DSAPIDevice(DSAPIRef pDSAPIRef, DSThird *pDSRgb);
		virtual ~DSAPIDevice();

		virtual OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSources);

		virtual oni::driver::StreamBase* createStream(OniSensorType streamSource);
		virtual void destroyStream(oni::driver::StreamBase* pStream);

		virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
		virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
		virtual OniBool isPropertySupported(int propertyId);

		virtual OniBool isCommandSupported(int commandId);

		virtual OniStatus tryManualTrigger();

		virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode);

	private:
		DSAPIRef m_pDSAPIRef;
		DSThird *m_pDSRgb;
		DSAPIStreamImpl* m_pDepthStream;
		DSAPIStreamImpl* m_pColorStream;
		//DSAPIStreamImpl* m_pIRStream;
		//DSAPIStreamImpl* m_pLeftStream;
		//DSAPIStreamImpl* m_pRightStream;
		int m_numSensors;
		LONGLONG m_perfCounter;
		OniSensorInfo m_sensors[10];
	};
}
#endif