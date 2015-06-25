#include "DSNIDevice.h"
#include "DepthDSNIStream.h"
#include "ColorDSNIStream.h"
#include "IRDSNIStream.h"
#include "DSAPI.h"
#include "DSAPIUtil.h"

using namespace dsni_device;
using namespace oni::driver;
#define DEFAULT_DEPTH_FPS 60
#define DEFAULT_COLOR_FPS 30

DSNIDevice::DSNIDevice(DSAPIRef pDSNIRef, DSThird *pDSRgb)
  : m_pDepthStream(NULL),
    m_pColorStream(NULL),
    //m_pLeftStream(NULL),
	//m_pRightStream(NULL),
    m_pDSNIRef(pDSNIRef),
	m_pDSRgb(pDSRgb)
{
  m_numSensors = 2;

  // Z
  m_sensors[0].sensorType = ONI_SENSOR_DEPTH;
  m_sensors[0].numSupportedVideoModes = 1;
  m_sensors[0].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
  m_sensors[0].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
  m_sensors[0].pSupportedVideoModes[0].fps = DEFAULT_DEPTH_FPS;
  m_sensors[0].pSupportedVideoModes[0].resolutionX = 480;
  m_sensors[0].pSupportedVideoModes[0].resolutionY = 360;

  // Third
  m_sensors[1].sensorType = ONI_SENSOR_COLOR;
  m_sensors[1].numSupportedVideoModes = 2;
  m_sensors[1].pSupportedVideoModes = XN_NEW_ARR(OniVideoMode, 1);
  m_sensors[1].pSupportedVideoModes[0].pixelFormat = ONI_PIXEL_FORMAT_RGB888;
  m_sensors[1].pSupportedVideoModes[0].fps         = DEFAULT_COLOR_FPS;
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

DSNIDevice::~DSNIDevice()
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

  if (m_pDSNIRef)
  {
	  m_pDSNIRef->stopCapture();
	  DSDestroy(m_pDSNIRef);
  }
}

OniStatus DSNIDevice::getSensorInfoList(OniSensorInfo** pSensors, int* numSensors)
{
  *numSensors = m_numSensors;
  *pSensors = m_sensors;
  return ONI_STATUS_OK;
}

StreamBase* DSNIDevice::createStream(OniSensorType sensorType)
{
  BaseDSNIStream* pImage = NULL;

  if (sensorType == ONI_SENSOR_COLOR)
  {
    if (m_pColorStream == NULL)
    {
      m_pColorStream = XN_NEW(DSNIStreamImpl, m_pDSNIRef, m_pDSRgb, sensorType, m_perfCounter);
    }
    pImage = XN_NEW(ColorDSNIStream, m_pColorStream);
  }
  else if (sensorType == ONI_SENSOR_DEPTH)
  {
    if (m_pDepthStream == NULL)
    {
      m_pDepthStream = XN_NEW(DSNIStreamImpl, m_pDSNIRef, m_pDSRgb, sensorType, m_perfCounter);
    }
    pImage = XN_NEW(DepthDSNIStream, m_pDepthStream);
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

void dsni_device::DSNIDevice::destroyStream(oni::driver::StreamBase* pStream)
{
  XN_DELETE(pStream);
}

OniStatus DSNIDevice::setProperty(int propertyId, const void* data, int dataSize)
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

OniStatus DSNIDevice::getProperty(int propertyId, void* data, int* pDataSize)
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

OniBool DSNIDevice::isPropertySupported(int propertyId)
{
  return (propertyId == ONI_DEVICE_PROPERTY_IMAGE_REGISTRATION);
}

OniBool DSNIDevice::isCommandSupported(int commandId)
{
  return ONI_STATUS_NOT_IMPLEMENTED;
}

OniStatus DSNIDevice::tryManualTrigger()
{
  return ONI_STATUS_NOT_IMPLEMENTED;
}

OniBool DSNIDevice::isImageRegistrationModeSupported(OniImageRegistrationMode mode)
{
  return (mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR || mode == ONI_IMAGE_REGISTRATION_OFF);
}
