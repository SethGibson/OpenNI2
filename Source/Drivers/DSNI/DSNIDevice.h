#ifndef _DSNI_DEVICE_H_
#define _DSNI_DEVICE_H_

#include "Driver\OniDriverAPI.h"
#include "DSNIStreamImpl.h"

namespace dsni_device
{
	class DSNIDevice : public oni::driver::DeviceBase
	{
	public:
		DSNIDevice(DSAPIRef pDSNIRef, DSThird *pDSRgb);
		virtual ~DSNIDevice();

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
		DSAPIRef m_pDSNIRef;
		DSThird *m_pDSRgb;
		DSNIStreamImpl* m_pDepthStream;
		DSNIStreamImpl* m_pColorStream;
		//DSNIStreamImpl* m_pIRStream;
		//DSNIStreamImpl* m_pLeftStream;
		//DSNIStreamImpl* m_pRightStream;
		int m_numSensors;
		LONGLONG m_perfCounter;
		OniSensorInfo m_sensors[10];
	};
}
#endif