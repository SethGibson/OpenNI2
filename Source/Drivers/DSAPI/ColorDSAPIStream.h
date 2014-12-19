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
#ifndef COLORDSAPISTREAM_H
#define COLORDSAPISTREAM_H

#include "BaseDSAPIStream.h"

//struct INuiSensor;
struct IDSAPISensor;
namespace dsapi_device
{

class ColorDSAPIStream : public BaseDSAPIStream
{
public:
	ColorDSAPIStream(DSAPIStreamImpl* pStreamImpl);
	
	virtual OniStatus start();

	//virtual void frameReceived(NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect);
	virtual void frameReceived(uint8_t *imageFrame);

	virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);

	virtual OniStatus setProperty(int propertyId, const void* data, int pDataSize);
	
	virtual OniBool isPropertySupported(int propertyId);

private:
	//void copyFrameRGB888(OniFrame* pFrame, NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect);
	//void copyFrameYUV422(OniFrame* pFrame, NUI_IMAGE_FRAME& imageFrame, NUI_LOCKED_RECT &LockedRect);
	void copyFrameRGB8888(OniFrame* pFrame, uint8_t *imageFrame);

};
} // namespace kinect_device

#endif // COLORKINECTSTREAM_H
