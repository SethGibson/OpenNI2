#include "DSAPIDevice.h"
#include "DepthDSAPIStream.h"
#include "ColorDSAPIStream.h"
#include "IRDSAPIStream.h"
#include "DSAPI.h"
#include "DSAPIUtil.h"

using namespace dsapi_device;
using namespace oni::driver;
#define DEFAULT_FPS 30

DSAPIDevice::DSAPIDevice(DSAPIRef pDSAPIRef, DSThird *pDSRgb)
  : m_pDepthStream(NULL),
    m_pColorStream(NULL),
    //m_pLeftStream(NULL),
	//m_pRightStream(NULL),
    m_pDSAPIRef(pDSAPIRef),
	m_pDSRgb(pDSRgb)
{
  m_numSensors = 2;

  // Z
  m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
  m_sensors[0].numSupportedVideoModes = 1;
  m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
  m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
  m_sensors[0].pSupportedVideoModes[0].fps = DEFAULT_FPS;
  m_sensors[0].pSupportedVideoModes[0].resolutionX = 480;
  m_sensors[0].pSupportedVideoModes[0].resolutionY = 360;

  // Third
  m_sensors[1].sensorType = ONI_SENSOR_COLOR;
  m_sensors[1].numSupportedVideoModes = 2;
  m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
  m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
  m_sensors[1].pSupportedVideoModes[0].fps         = DEFAULT_FPS;
  m_sensors[1].pSupportedVideoModes[0].resolutionX = 640;
  m_sensors[1].pSupportedVideoModes[0].resolutionY = 480;

  /*
  // Left
  m_sensors[2].sensorType = ONI_SENSOR_IR;
  m_sensors[2].numSupportedVideoModes = 1;
  m_sensors[2].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
  m_sensors[2].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
  m_sensors[2].pSupportedVideoModes[0].fps = DEFAULT_FPS;
  m_sensors[2].pSupportedVideoModes[0].resolutionX = 480;
  m_sensors[2].pSupportedVideoModes[0].resolutionY = 360;

  // Right
  m_sensors[3].sensorType = ONI_SENSOR_IR;
  m_sensors[3].numSupportedVideoModes = 1;
  m_sensors[3].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
  m_sensors[3].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
  m_sensors[3].pSupportedVideoModes[0].fps = DEFAULT_FPS;
  m_sensors[3].pSupportedVideoModes[0].resolutionX = 480;
  m_sensors[3].pSupportedVideoModes[0].resolutionY = 360;
  */
  LARGE_INTEGER qpc = {0};
  if (QueryPerformanceCounter(&qpc))
  {
      m_perfCounter = qpc.QuadPart;
  }
}

DSAPIDevice::~DSAPIDevice()
{
  if (m_pDepthStream != NULL) {
    XN_DELETE(m_pDepthStream);
  }

  if (m_pColorStream!= NULL) {
    XN_DELETE(m_pColorStream);
  }

  /*
  if (m_pLeftStream!= NULL) {
    XN_DELETE(m_pLeftStream);
  }
  if (m_pRightStream != NULL) {
	  XN_DELETE(m_pRightStream);
  }*/

  if (m_pDSAPIRef)
  {
	  m_pDSAPIRef->stopCapture();
	  DSDestroy(m_pDSAPIRef);
  }
}

OniStatus DSAPIDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
  *numSensors = m_numSensors;
  *pSensors = m_sensors;
  return ONI_STATUS_OK;
}

StreamBase* DSAPIDevice::createStream(OniSensorType sensorType)
{
  BaseDSAPIStream* pImage = NULL;

  if (sensorType == ONI_SENSOR_COLOR)
  {
    if (m_pColorStream == NULL)
    {
      m_pColorStream = XN_NEW(DSAPIStreamImpl, m_pDSAPIRef, m_pDSRgb, sensorType, m_perfCounter);
    }
    pImage = XN_NEW(ColorDSAPIStream, m_pColorStream);
  }
  else if (sensorType == ONI_SENSOR_DEPTH)
  {
    if (m_pDepthStream == NULL)
    {
      m_pDepthStream = XN_NEW(DSAPIStreamImpl, m_pDSAPIRef, m_pDSRgb, sensorType, m_perfCounter);
    }
    pImage = XN_NEW(DepthDSAPIStream, m_pDepthStream);
  }
  /*
  else if (sensorType ==  ONI_SENSOR_IR)
  {
    if (m_pIRStream == NULL)
    {
      m_pIRStream = XN_NEW(Kinect2StreamImpl, m_pKinectSensor, sensorType, m_perfCounter);
    }
    pImage = XN_NEW(IRKinect2Stream, m_pIRStream);
  }*/
  return pImage;
}

void dsapi_device::DSAPIDevice::destroyStream(oni::driver::StreamBase* pStream)
{
  XN_DELETE(pStream);
}

OniStatus DSAPIDevice::setProperty(int propertyId, const void* data, int dataSize)
{
  switch (propertyId) {
  case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
    if (dataSize == sizeof(OniImageRegistrationMode)) {
      if (m_pDepthStream) {
        OniImageRegistrationMode* pMode = (OniImageRegistrationMode*)data;
        m_pDepthStream->setImageRegistrationMode(*pMode);
        return ONI_STATUS_OK;
      }
      else {
        return ONI_STATUS_ERROR;
      }
    }
    else {
      printf("Unexpected size: %d != %d\n", dataSize, sizeof(OniImageRegistrationMode));
      return ONI_STATUS_ERROR;
    }
  }
  return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus DSAPIDevice::getProperty(int propertyId, void* data, int* pDataSize)
{
  switch (propertyId) {
  case ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION:
    if (*pDataSize == sizeof(OniImageRegistrationMode)) {
      if (m_pDepthStream) {
        OniImageRegistrationMode* pMode = (OniImageRegistrationMode*)data;
        *pMode = m_pDepthStream->getImageRegistrationMode();
        return ONI_STATUS_OK;
      } else {
        return ONI_STATUS_ERROR;
      }
    }
    else {
      printf("Unexpected size: %d != %d\n", *pDataSize, sizeof(OniImageRegistrationMode));
      return ONI_STATUS_ERROR;
    }
  }
  return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool DSAPIDevice::isPropertySupported(int propertyId)
{
  return (propertyId == ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION);
}

OniBool DSAPIDevice::isCommandSupported(int commandId)
{
  return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus DSAPIDevice::tryManualTrigger()
{
  return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool DSAPIDevice::isImageRegistrationModeSupported(OniImageRegistrationMode mode)
{
  return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
}
