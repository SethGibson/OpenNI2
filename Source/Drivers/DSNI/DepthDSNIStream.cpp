#include "DepthDSNIStream.h"

#include "DSNIStreamImpl.h"

using namespace oni::driver;
using namespace dsni_device;
#define DEFAULT_FPS 60

#define DEVICE_MAX_DEPTH_VAL 5000
#define FILTER_RELIABLE_DEPTH_VALUE(VALUE) (((VALUE) < DEVICE_MAX_DEPTH_VAL) ? (VALUE) : 0)

DepthDSNIStream::DepthDSNIStream(DSNIStreamImpl* pStreamImpl)
  : BaseDSNIStream(pStreamImpl)
{
  m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
  m_videoMode.fps = DEFAULT_FPS;
  m_videoMode.resolutionX = 480;
  m_videoMode.resolutionY = 360;
  m_colorSpaceCoords = new ColorSpacePoint[480*360];
  m_registeredDepthMap = new UINT16[480*360];
}

DepthDSNIStream::~DepthDSNIStream()
{
  delete[] m_colorSpaceCoords;
  delete[] m_registeredDepthMap;
}

void DepthDSNIStream::frameReady(void* data, int width, int height, double timestamp)
{
  OniFrame* pFrame = getServices().acquireFrame();
  pFrame->videoMode.resolutionY = m_videoMode.resolutionY;
  pFrame->videoMode.resolutionX = m_videoMode.resolutionX;
  pFrame->croppingEnabled = m_cropping.enabled;
  if (m_cropping.enabled)
  {
    pFrame->width = m_cropping.width;
    pFrame->height = m_cropping.height;
    pFrame->cropOriginX = m_cropping.originX;
    pFrame->cropOriginY = m_cropping.originY;
  }
  else {
    pFrame->cropOriginX = 0;
    pFrame->cropOriginY = 0;
    pFrame->width = m_videoMode.resolutionX;
    pFrame->height = m_videoMode.resolutionY;
  }
  pFrame->dataSize = pFrame->height * pFrame->width * 2;
  pFrame->stride = pFrame->width * 2;
  pFrame->videoMode.pixelFormat = m_videoMode.pixelFormat;
  pFrame->videoMode.fps = m_videoMode.fps;
  pFrame->sensorType = ONI_SENSOR_DEPTH;
  pFrame->frameIndex = m_frameIdx++;
  pFrame->timestamp = static_cast<int>(timestamp);

  UINT16* data_in = reinterpret_cast<UINT16*>(data);
  if (m_pStreamImpl->getImageRegistrationMode() == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) {
    copyDepthPixelsWithImageRegistration(data_in, width, height, pFrame);
  } else {
    copyDepthPixelsStraight(data_in, width, height, pFrame);
  }

  raiseNewFrame(pFrame);
  getServices().releaseFrame(pFrame);
}

OniStatus DepthDSNIStream::getProperty(int propertyId, void* data, int* pDataSize)
{
  OniStatus status = ONI_STATUS_NOT_SUPPORTED;
  switch (propertyId)
  {
  case ONI_STREAM_PROPERTY_MAX_VALUE:
    {
      XnInt * val = (XnInt *)data;
      *val = DEVICE_MAX_DEPTH_VAL;
      status = ONI_STATUS_OK;
      break;
    }
  case ONI_STREAM_PROPERTY_MIRRORING:
    {
      XnBool * val = (XnBool *)data;
      *val = TRUE;
      status = ONI_STATUS_OK;
      break;
    }
  default:
    status = BaseDSNIStream::getProperty(propertyId, data, pDataSize);
    break;
  }

  return status;
}

OniBool DepthDSNIStream::isPropertySupported(int propertyId)
{
  OniBool status = FALSE;
  switch (propertyId)
  {
  case ONI_STREAM_PROPERTY_MAX_VALUE:
  case ONI_STREAM_PROPERTY_MIRRORING:
    status = TRUE;
  default:
    status = BaseDSNIStream::isPropertySupported(propertyId);
    break;
  }
  return status;
}

void DepthDSNIStream::notifyAllProperties()
{
  XnInt nInt;
  int size = sizeof(nInt);
  getProperty(ONI_STREAM_PROPERTY_MAX_VALUE, &nInt, &size);
  raisePropertyChanged(ONI_STREAM_PROPERTY_MAX_VALUE, &nInt, size);

  BaseDSNIStream::notifyAllProperties();
}

void DepthDSNIStream::copyDepthPixelsStraight(const UINT16* data_in, int width, int height, OniFrame* pFrame)
{
  // Copy the depth pixels to OniDriverFrame
  // with applying cropping but NO depth-to-image registration.

  const int xStride = width/m_videoMode.resolutionX;
  const int yStride = height/m_videoMode.resolutionY;
  const int frameX = pFrame->cropOriginX * xStride;
  const int frameY = pFrame->cropOriginY * yStride;
  const int frameWidth = pFrame->width * xStride;
  const int frameHeight = pFrame->height * yStride;

  unsigned short* data_out = (unsigned short*) pFrame->data;
  for (int y = frameY; y < frameY + frameHeight; y += yStride) {
    for (int x = frameX; x < frameX + frameWidth; x += xStride) {
      unsigned short* iter = const_cast<unsigned short*>(data_in + (y*width + x));
      *data_out = FILTER_RELIABLE_DEPTH_VALUE(*iter);
      data_out++;
    }
  }
}

void DepthDSNIStream::copyDepthPixelsWithImageRegistration(const UINT16* data_in, int width, int height, OniFrame* pFrame)
{
  // Copy the depth pixels to OniDriverFrame
  // with applying cropping and depth-to-image registration.

	const int xStride = width/m_videoMode.resolutionX;
	const int yStride = height/m_videoMode.resolutionY;
	const int frameX = pFrame->cropOriginX * xStride;
	const int frameY = pFrame->cropOriginY * yStride;
	const int frameWidth = pFrame->width * xStride;
	const int frameHeight = pFrame->height * yStride;
	const float xFactor = static_cast<float>(width)/640.0f;
	const float yFactor = static_cast<float>(height)/480.0f;

	DSCalibIntrinsicsRectified cZIntrins = m_pStreamImpl->getZIntrinsicsRect();
	DSCalibIntrinsicsRectified cRgbIntrins = m_pStreamImpl->getRgbIntrinsicsRect();
	const double *cZToThirdTrans = m_pStreamImpl->getZToThirdTrans();

	unsigned short* data_out = (unsigned short*)m_registeredDepthMap;
	xnOSMemSet(data_out, 0, width*height * 2);
	const ColorSpacePoint* mappedCoordsIter = m_colorSpaceCoords;

	for (int y = 0; y < m_videoMode.resolutionY; ++y)
	{
		for (int x = 0; x < m_videoMode.resolutionX; ++x)
		{
			float cZImage[3] = { static_cast<float>(x),
									static_cast<float>(y),
									static_cast<float>(data_in[y*480+x]) };
			float cThird[2]{0};

			DSTransformFromZImageToRectThirdImage(cZIntrins, cZToThirdTrans, cRgbIntrins, cZImage, cThird);
			const float fX = cThird[0];
			const float fY = cThird[1];
			const int cx = static_cast<int>(fX + 0.5f);
			const int cy = static_cast<int>(fY + 0.5f);
			if (cx >= 0 && cy >= 0 && cx < width && cy < height)
			{
				unsigned short* iter = const_cast<unsigned short*>(data_in + (y*width + x));
				const unsigned short d = FILTER_RELIABLE_DEPTH_VALUE(*iter);
				unsigned short* const p = data_out + cx + cy * width;
				if (*p == 0 || *p > d)
					*p = d;
			}
		}
	}

	data_out = (unsigned short*)pFrame->data;
	for (int y = frameY; y < frameY + frameHeight; y += yStride)
	{
		for (int x = frameX; x < frameX + frameWidth; x += xStride)
		{
			unsigned short* iter = const_cast<unsigned short*>(m_registeredDepthMap + (y*width + x));
			if (*iter == 0)
			{
				unsigned short davg = 0;
				int dw = 0;
				for (int ky = max(y - 1, 0); ky <= y + 1 && ky < height; ky++)
				{
					unsigned short* kiter = const_cast<unsigned short*>(m_registeredDepthMap + (ky*width + x));
					if (*kiter != 0)
					{
						davg += *kiter;
						dw += abs(ky - y);
					}
				}
				*data_out = davg;
				if (dw)
				{
					*data_out /= dw;
				}
			}
			else
			{
				*data_out = FILTER_RELIABLE_DEPTH_VALUE(*iter);
			}
			data_out++;
		}
	}
}
