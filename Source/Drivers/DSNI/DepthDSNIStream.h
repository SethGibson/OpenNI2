#ifndef _DEPTH_DSNI_STREAM_H_
#define _DEPTH_DSNI_STREAM_H_

#include "BaseDSNIStream.h"

namespace dsni_device
{
	struct ColorSpacePoint
	{
		float X;
		float Y;
	};

	class DepthDSNIStream : public BaseDSNIStream
	{
	public:
		DepthDSNIStream(DSNIStreamImpl* pStreamImpl);
		virtual ~DepthDSNIStream();

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