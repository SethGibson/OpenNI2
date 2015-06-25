#ifndef _DEPTH_DSAPI_STREAM_H_
#define _DEPTH_DSAPI_STREAM_H_

#include "BaseDSAPIStream.h"

namespace dsapi_device
{
	struct ColorSpacePoint
	{
		float X;
		float Y;
	};

	class DepthDSAPIStream : public BaseDSAPIStream
	{
	public:
		DepthDSAPIStream(DSAPIStreamImpl* pStreamImpl);
		virtual ~DepthDSAPIStream();

		virtual void frameReady(void* data, int width, int height, double timestamp);

		virtual OniStatus getProperty(int propertyId, void* data, int* pDataSize);
		virtual OniBool isPropertySupported(int propertyId);
		virtual void notifyAllProperties();

	private:
		void copyDepthPixelsStraight(const UINT16* data_in, int width, int height, OniFrame* pFrame);
		void copyDepthPixelsWithImageRegistration(const UINT16* data_in, int width, int height, OniFrame* pFrame);

	private:
		ColorSpacePoint* m_colorSpaceCoords;
		UINT16* m_registeredDepthMap;
	};
}
#endif