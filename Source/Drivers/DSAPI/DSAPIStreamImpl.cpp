#include "DSAPIStreamImpl.h"
#include "BaseDSAPIStream.h"

#include "DSAPI.h"
#include "DSAPIUtil.h"

using namespace oni::driver;
using namespace dsapi_device;
using namespace xnl;

#define DEFAULT_FPS 30

DSAPIStreamImpl::DSAPIStreamImpl(DSAPIRef pDSAPIRef, DSThird *pDSThird, OniSensorType sensorType, LONGLONG basePerfCounter)
  : m_pDSAPIRef(pDSAPIRef),
	m_pDSThird(pDSThird),
    m_sensorType(sensorType),
    m_imageRegistrationMode(ONI_IMAGE_REGISTRATION_OFF),
    m_running(FALSE),
    m_perfCounter(basePerfCounter),
    m_perfFreq(1.0)
{

	pDSAPIRef->getCalibIntrinsicsZ(m_pZIntrins);
	pDSThird->getCalibIntrinsicsRectThird(m_pRgbIntrins);
	pDSThird->getCalibExtrinsicsZToRectThird(m_pZtoThirdTrans);

	m_pFrameBuffer.color = NULL;
	m_pFrameBuffer.depth = NULL;
	//m_pFrameBuffer.infrared = NULL;
	createFrameBuffer();

	/*
	m_pFrameReader.color = NULL;
	m_pFrameReader.depth = NULL;
	m_pFrameReader.infrared = NULL;
	openFrameReader();*/

	LARGE_INTEGER qpf = {0};
	if (QueryPerformanceFrequency(&qpf))
	{
		m_perfFreq = double(qpf.QuadPart)/1000000.0;
	}

	setDefaultVideoMode();
}

DSAPIStreamImpl::~DSAPIStreamImpl()
{
  if (m_running) {
    m_running = FALSE;
    xnOSWaitForThreadExit(m_threadHandle, INFINITE);
    xnOSCloseThread(&m_threadHandle);
  }

  //closeFrameReader();
  destroyFrameBuffer();
}

void DSAPIStreamImpl::addStream(BaseDSAPIStream* stream)
{
  m_streamList.AddLast(stream);
}

void DSAPIStreamImpl::removeStream(BaseDSAPIStream* stream)
{
  m_streamList.Remove(stream);
}

unsigned int DSAPIStreamImpl::getStreamCount()
{
  return m_streamList.Size();
}

void DSAPIStreamImpl::setVideoMode(OniVideoMode* videoMode)
{
  m_videoMode.fps = videoMode->fps;
  m_videoMode.pixelFormat = videoMode->pixelFormat;
  m_videoMode.resolutionX = videoMode->resolutionX;
  m_videoMode.resolutionY = videoMode->resolutionY;
}

OniStatus DSAPIStreamImpl::start()
{
  if (m_running != TRUE) {
    XnStatus nRetVal = xnOSCreateThread(threadFunc, this, &m_threadHandle);
    if (nRetVal != XN_STATUS_OK) {
      return ONI_STATUS_ERROR;
    }
    return ONI_STATUS_OK;
  }
  else {
    return ONI_STATUS_OK;
  }
}

void DSAPIStreamImpl::stop()
{
  if (m_running == true) {
    List<BaseDSAPIStream*>::Iterator iter = m_streamList.Begin();
    while( iter != m_streamList.End()) {
      if (((BaseDSAPIStream*)(*iter))->isRunning()) {
        return;
      }
      ++iter;
    }
    m_running = false;
    xnOSWaitForThreadExit(m_threadHandle, INFINITE);
    xnOSCloseThread(&m_threadHandle);
  }
}

void DSAPIStreamImpl::setSensorType(OniSensorType sensorType)
{
  if (m_sensorType != sensorType) {
    //closeFrameReader();
    destroyFrameBuffer();

    m_sensorType = sensorType;

    createFrameBuffer();
    //openFrameReader();

    setDefaultVideoMode();
  }
}

void DSAPIStreamImpl::mainLoop()
{
	m_running = TRUE;
	while (m_running)
	{
		int width, height;
		void* data = populateFrameBuffer(width, height);

		LARGE_INTEGER qpc = {0};
		QueryPerformanceCounter(&qpc);
		double timestamp = static_cast<double>(qpc.QuadPart - m_perfCounter)/m_perfFreq;

		List<BaseDSAPIStream*>::ConstIterator iter = m_streamList.Begin();
		while( iter != m_streamList.End())
		{
			if (((BaseDSAPIStream*)(*iter))->isRunning())
			{
				((BaseDSAPIStream*)(*iter))->frameReady(data, width, height, timestamp);
			}
			++iter;
		}
	}
	return;
}

XnDouble DSAPIStreamImpl::getHorizontalFov()
{
	float cHFOV, cVFOV;
	if (m_sensorType == ONI_SENSOR_DEPTH && m_imageRegistrationMode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR)
		DSFieldOfViewsFromIntrinsicsRect(m_pZIntrins, cHFOV, cVFOV);
	else
		DSFieldOfViewsFromIntrinsicsRect(m_pRgbIntrins, cHFOV, cVFOV);
	return cHFOV;
}

XnDouble DSAPIStreamImpl::getVerticalFov()
{
	float cHFOV, cVFOV;
	if (m_sensorType == ONI_SENSOR_DEPTH && m_imageRegistrationMode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR)
		DSFieldOfViewsFromIntrinsicsRect(m_pZIntrins, cHFOV, cVFOV);
	else
		DSFieldOfViewsFromIntrinsicsRect(m_pRgbIntrins, cHFOV, cVFOV);
	return cVFOV;
}

void DSAPIStreamImpl::setDefaultVideoMode()
{
  switch (m_sensorType)
  {
  case ONI_SENSOR_COLOR:
    m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
    m_videoMode.fps         = DEFAULT_FPS;
    m_videoMode.resolutionX = 640;
    m_videoMode.resolutionY = 480;
    break;
  case ONI_SENSOR_DEPTH:
    m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
    m_videoMode.fps         = DEFAULT_FPS;
    m_videoMode.resolutionX = 480;
    m_videoMode.resolutionY = 360;
    break;
	/*
  case ONI_SENSOR_IR:
    m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_GRAY16;
    m_videoMode.fps         = DEFAULT_FPS;
    m_videoMode.resolutionX = 480;
    m_videoMode.resolutionY = 360;
    break;*/
  default:
    break;
  }
}

/*
IFrameDescription* DSAPIStreamImpl::getFrameDescription(OniSensorType sensorType)
{
  if (!m_pDSAPIRef) {
    return NULL;
  }

  IFrameDescription* frameDescription = NULL;
  if (sensorType == ONI_SENSOR_COLOR) {
    IColorFrameSource* frameSource = NULL;
    HRESULT hr = m_pDSAPIRef->get_ColorFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
      hr = frameSource->get_FrameDescription(&frameDescription);
      if (FAILED(hr) && frameDescription) {
        frameDescription->Release();
        frameDescription = NULL;
      }
    }
    if (frameSource) {
      frameSource->Release();
    }
  }
  else if (sensorType == ONI_SENSOR_DEPTH) {
    IDepthFrameSource* frameSource = NULL;
    HRESULT hr = m_pDSAPIRef->get_DepthFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
      hr = frameSource->get_FrameDescription(&frameDescription);
      if (FAILED(hr) && frameDescription) {
        frameDescription->Release();
        frameDescription = NULL;
      }
    }
    if (frameSource) {
      frameSource->Release();
    }
  }
  else { // ONI_SENSOR_IR
    IInfraredFrameSource* frameSource = NULL;
    HRESULT hr = m_pDSAPIRef->get_InfraredFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
      hr = frameSource->get_FrameDescription(&frameDescription);
      if (FAILED(hr) && frameDescription) {
        frameDescription->Release();
        frameDescription = NULL;
      }
    }
    if (frameSource) {
      frameSource->Release();
    }
  }

  return frameDescription;
}*/

void DSAPIStreamImpl::createFrameBuffer()
{
  if (m_sensorType == ONI_SENSOR_COLOR && !m_pFrameBuffer.color) {
    m_pFrameBuffer.color = new uint8_t[640*480];
  }
  else if (m_sensorType == ONI_SENSOR_DEPTH && !m_pFrameBuffer.depth) {
    m_pFrameBuffer.depth = new uint16_t[480*360];
  }
  /*
  else if (!m_pFrameBuffer.infrared) { // ONI_SENSOR_IR
    m_pFrameBuffer.infrared = new UINT16[512*424];
  }*/
}

void DSAPIStreamImpl::destroyFrameBuffer()
{
  if (m_sensorType == ONI_SENSOR_COLOR && m_pFrameBuffer.color) {
    delete[] m_pFrameBuffer.color;
    m_pFrameBuffer.color = NULL;
  }
  else if (m_sensorType == ONI_SENSOR_DEPTH && m_pFrameBuffer.depth) {
    delete[] m_pFrameBuffer.depth;
    m_pFrameBuffer.depth = NULL;
  }
  /*
  else if (m_pFrameBuffer.infrared) { // ONI_SENSOR_IR
    delete[] m_pFrameBuffer.infrared;
    m_pFrameBuffer.infrared = NULL;
  }*/
}

/*
void DSAPIStreamImpl::openFrameReader()
{
  if (!m_pDSAPIRef) {
    return;
  }

  if (m_sensorType == ONI_SENSOR_COLOR && !m_pFrameReader.color) {
    IColorFrameSource* frameSource = NULL;
    HRESULT hr = m_pDSAPIRef->get_ColorFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
      hr = frameSource->OpenReader(&m_pFrameReader.color);
      if (FAILED(hr) && m_pFrameReader.color) {
        m_pFrameReader.color->Release();
        m_pFrameReader.color = NULL;
      }
    }
    if (frameSource) {
      frameSource->Release();
    }
  }
  else if (m_sensorType == ONI_SENSOR_DEPTH && !m_pFrameReader.depth) {
    IDepthFrameSource* frameSource = NULL;
    HRESULT hr = m_pDSAPIRef->get_DepthFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
      hr = frameSource->OpenReader(&m_pFrameReader.depth);
      if (FAILED(hr) && m_pFrameReader.depth) {
        m_pFrameReader.depth->Release();
        m_pFrameReader.depth = NULL;
      }
    }
    if (frameSource) {
      frameSource->Release();
    }
  }
  else if(!m_pFrameReader.infrared) { // ONI_SENSOR_IR
    IInfraredFrameSource* frameSource = NULL;
    HRESULT hr = m_pDSAPIRef->get_InfraredFrameSource(&frameSource);
    if (SUCCEEDED(hr)) {
      hr = frameSource->OpenReader(&m_pFrameReader.infrared);
      if (FAILED(hr) && m_pFrameReader.infrared) {
        m_pFrameReader.infrared->Release();
        m_pFrameReader.infrared = NULL;
      }
    }
    if (frameSource) {
      frameSource->Release();
    }
  }
}

void DSAPIStreamImpl::closeFrameReader()
{
  if (m_sensorType == ONI_SENSOR_COLOR && m_pFrameReader.color) {
    m_pFrameReader.color->Release();
    m_pFrameReader.color = NULL;
  }
  else if (m_sensorType == ONI_SENSOR_DEPTH && m_pFrameReader.depth) {
    m_pFrameReader.depth->Release();
    m_pFrameReader.depth = NULL;
  }
  else if (m_pFrameReader.infrared) { // ONI_SENSOR_IR
    m_pFrameReader.infrared->Release();
    m_pFrameReader.infrared = NULL;
  }
}*/

// Frame has been grab()'bed
void* DSAPIStreamImpl::populateFrameBuffer(int& buffWidth, int& buffHeight)
{
	buffWidth = 0;
	buffHeight = 0;
	if (m_pDSAPIRef->grab())
	{
		if (m_sensorType == ONI_SENSOR_COLOR)
		{
			if (m_pDSThird&&m_pFrameBuffer.color)
			{
				buffWidth = 640;
				buffHeight = 480;

				memcpy(m_pFrameBuffer.color, (uint8_t*)m_pDSThird->getThirdImage(), size_t(640 * 480 * 3));

				return reinterpret_cast<void*>(m_pFrameBuffer.color);
			}
		}

		else if (m_sensorType == ONI_SENSOR_DEPTH)
		{
			if (m_pDSAPIRef && m_pFrameBuffer.depth)
			{
				buffWidth = 480;
				buffHeight = 360;

				memcpy(m_pFrameBuffer.depth, (uint16_t *)m_pDSAPIRef->getZImage(), size_t(480 * 360 * sizeof(uint16_t)));

				return reinterpret_cast<void*>(m_pFrameBuffer.depth);
			}
		}
	}
	return NULL;

  /*
  else { // ONI_SENSOR_IR
    if (m_pFrameReader.infrared && m_pFrameBuffer.infrared) {
      buffWidth = 512;
      buffHeight = 424;

      IInfraredFrame* frame = NULL;
      HRESULT hr = m_pFrameReader.infrared->AcquireLatestFrame(&frame);
      if (SUCCEEDED(hr)) {
        UINT16* data;
        UINT bufferSize;
        frame->AccessUnderlyingBuffer(&bufferSize, &data);
        memcpy(m_pFrameBuffer.infrared, data, 512*424*sizeof(UINT16));
      }
      if (frame) {
        frame->Release();
      }

      return reinterpret_cast<void*>(m_pFrameBuffer.infrared);
    }
  }*/
}

XN_THREAD_PROC DSAPIStreamImpl::threadFunc(XN_THREAD_PARAM pThreadParam)
{
  DSAPIStreamImpl* pStream = (DSAPIStreamImpl*)pThreadParam;
  pStream->mainLoop();
  XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}
