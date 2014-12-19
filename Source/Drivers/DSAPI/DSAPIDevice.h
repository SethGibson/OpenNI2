/*****************************************************************************
*                                                                            *
*  OpenNI 2.x Alpha                                                          *
*  Copyright (C) 2012 PrimeSense Ltd.                                        *
*                                                                            *
*  This file is part of OpenNI.                                              *
*                                                                            *
*  Licensed under the Apache License, Version 2.0 (the "License");           *
*  you may not use this file except in compliance with the License.          *
*  You may obtain a copy of the License at                                   *
*                                                                            *
*      http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                            *
*  Unless required by applicable law or agreed to in writing, software       *
*  distributed under the License is distributed on an "AS IS" BASIS,         *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  *
*  See the License for the specific language governing permissions and       *
*  limitations under the License.                                            *
*                                                                            *
*****************************************************************************/
#ifndef DSAPIDEVICE_H
#define DSAPIDEVICE_H

#include "Driver\OniDriverAPI.h"
#include "DSAPIStreamImpl.h"
#include "XnLib.h"
#include "XnHash.h"
#include "XnEvent.h"

//struct 	INuiSensor;
//struct IDSAPISensor;
class DSAPI;

namespace dsapi_device {

class DSAPIDevice : public oni::driver::DeviceBase 
{
public:
	DSAPIDevice(DSAPI *pDSAPISensor);
	virtual ~DSAPIDevice();

	virtual OniStatus getSensorInfoList(OniSensorInfo** pSensors, int* numSources);

	virtual oni::driver::StreamBase* createStream(OniSensorType streamSource);
	virtual void destroyStream(oni::driver::StreamBase* pStream);

	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
	virtual OniBool isPropertySupported(int propertyId);
	virtual OniBool isCommandSupported(int commandId) ;
	virtual OniStatus tryManualTrigger();

	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode);

private:
	DSAPI *m_pDSAPISensor;
	DSAPIStreamImpl* m_pDepthStream;
	DSAPIStreamImpl* m_pColorStream;
	int m_numSensors;
	OniSensorInfo m_sensors[10];	
};
} // namespace kinect_device
#endif // KINECTDEVICE_H
