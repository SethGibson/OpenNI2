#include "DSNIStreamImpl.h"
#include "BaseDSNIStream.h"

#include "DSAPI.h"
#include "DSAPIUtil.h"

using namespace oni::driver;
using namespace dsni_device;
using namespace xnl;

#define DEFAULT_DEPTH_FPS 60
#define DEFAULT_COLOR_FPS 30

DSNIStreamImpl::DSNIStreamImpl(DSAPIRef pDSNIRef, DSThird *pDSThird, OniSensorType sensorType, LONGLONG basePerfCounter)
  : m_pDSNIRef(pDSNIRef),
	m_pDSThird(pDSThird),
    m_sensorType(sensorType),
    m_imageRegistrationMode(ONI_IMAGE_REGISTRATION_OFF),
    m_running(FALSE),
    m_perfCounter(basePerfCounter),
    m_perfFreq(1.0)
{

	pDSNIRef->getCalibIntrinsicsZ(m_pZIntrins);
	pDSThird->getCalibIntrinsicsRectThird(m_pRgbIntrins);
	pDSThird->getCalibExtrinsicsZToRectThird(m_pZtoThirdTrans);

	m_pColorBuffer = NULL;
	m_pDepthBuffer = NULL;
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

DSNIStreamImpl::~DSNIStreamImpl()
{
  if (m_running) {
    m_running = FALSE;
    xnOSWaitForThreadExit(m_threadHandle, INFINITE);
    xnOSCloseThread(&m_threadHandle);
  }

  //closeFrameReader();
  destroyFrameBuffer();
}

void DSNIStreamImpl::addStream(BaseDSNIStream* stream)
{
  m_streamList.AddLast(stream);
}

void DSNIStreamImpl::removeStream(BaseDSNIStream* stream)
{
  m_streamList.Remove(stream);
}

unsigned int DSNIStreamImpl::getStreamCount()
{
  return m_streamList.Size();
}

void DSNIStreamImpl::setVideoMode(OniVideoMode* videoMode)
{
  m_videoMode.fps = videoMode->fps;
  m_videoMode.pixelFormat = videoMode->pixelFormat;
  m_videoMode.resolutionX = videoMode->resolutionX;
  m_videoMode.resolutionY = videoMode->resolutionY;
}

OniStatus DSNIStreamImpl::start()
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

void DSNIStreamImpl::stop()
{
  if (m_running == true) {
    List<BaseDSNIStream*>::Iterator iter = m_streamList.Begin();
    while( iter != m_streamList.End()) {
      if (((BaseDSNIStream*)(*iter))->isRunning()) {
        return;
      }
      ++iter;
    }
    m_running = false;
    xnOSWaitForThreadExit(m_threadHandle, INFINITE);
    xnOSCloseThread(&m_threadHandle);
  }
}

void DSNIStreamImpl::setSensorType(OniSensorType sensorType)
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

void DSNIStreamImpl::mainLoop()
{
	m_running = TRUE;


	while (m_running)
	{
		int width, height;

		void* data = populateFrameBuffer(width, height);

		LARGE_INTEGER qpc = {0};
		QueryPerformanceCounter(&qpc);
		double timestamp = static_cast<double>(qpc.QuadPart - m_perfCounter)/m_perfFreq;

		List<BaseDSNIStream*>::ConstIterator iter = m_streamList.Begin();
		while( iter != m_streamList.End())
		{
			if (((BaseDSNIStream*)(*iter))->isRunning())
			{
				((BaseDSNIStream*)(*iter))->frameReady(data, width, height, timestamp);
			}
			++iter;
		}
	}

	return;
}

XnDouble DSNIStreamImpl::getHorizontalFov()
{
	float cHFOV, cVFOV;
	if (m_sensorType == ONI_SENSOR_DEPTH && m_imageRegistrationMode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR)
		DSFieldOfViewsFromIntrinsicsRect(m_pZIntrins, cHFOV, cVFOV);
	else
		DSFieldOfViewsFromIntrinsicsRect(m_pRgbIntrins, cHFOV, cVFOV);
	return cHFOV;
}

XnDouble DSNIStreamImpl::getVerticalFov()
{
	float cHFOV, cVFOV;
	if (m_sensorType == ONI_SENSOR_DEPTH && m_imageRegistrationMode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR)
		DSFieldOfViewsFromIntrinsicsRect(m_pZIntrins, cHFOV, cVFOV);
	else
		DSFieldOfViewsFromIntrinsicsRect(m_pRgbIntrins, cHFOV, cVFOV);
	return cVFOV;
}

void DSNIStreamImpl::setDefaultVideoMode()
{
  switch (m_sensorType)
  {
  case ONI_SENSOR_COLOR:
    m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_RGB888;
    m_videoMode.fps         = DEFAULT_COLOR_FPS;
    m_videoMode.resolutionX = 640;
    m_videoMode.resolutionY = 480;
    break;
  case ONI_SENSOR_DEPTH:
    m_videoMode.pixelFormat = ONI_PIXEL_FORMAT_DEPTH_1_MM;
    m_videoMode.fps         = DEFAULT_DEPTH_FPS;
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

void DSNIStreamImpl::createFrameBuffer()
{
  if (m_sensorType == ONI_SENSOR_COLOR && !m_pColorBuffer)
  {
    m_pColorBuffer = new uint8_t[640*480*3];
  }
  else if (m_sensorType == ONI_SENSOR_DEPTH && !m_pDepthBuffer) {
    m_pDepthBuffer = new uint16_t[480*360];
  }
  /*
  else if (!m_pFrameBuffer.infrared) { // ONI_SENSOR_IR
    m_pFrameBuffer.infrared = new UINT16[512*424];
  }*/
}

void DSNIStreamImpl::destroyFrameBuffer()
{
  if (m_sensorType == ONI_SENSOR_COLOR && m_pColorBuffer) {
    delete[] m_pColorBuffer;
    m_pColorBuffer = NULL;
  }
  else if (m_sensorType == ONI_SENSOR_DEPTH && m_pDepthBuffer) {
    delete[] m_pDepthBuffer;
    m_pDepthBuffer = NULL;
  }
  /*
  else if (m_pFrameBuffer.infrared) { // ONI_SENSOR_IR
    delete[] m_pFrameBuffer.infrared;
    m_pFrameBuffer.infrared = NULL;
  }*/
}

// Frame has been grab()'bed
void* DSNIStreamImpl::populateFrameBuffer(int& buffWidth, int& buffHeight)
{
	buffWidth = 0;
	buffHeight = 0;

	if (m_pDSNIRef->grab())
	{
		if (m_sensorType == ONI_SENSOR_COLOR)
		{
			if (m_pDSThird&&m_pColorBuffer)
			{
				buffWidth = 640;
				buffHeight = 480;

				memcpy(m_pColorBuffer, (uint8_t*)m_pDSThird->getThirdImage(), size_t(640 * 480 * 3));

				return reinterpret_cast<void*>(m_pColorBuffer);
			}
			else
				return NULL;
		}

		else if (m_sensorType == ONI_SENSOR_DEPTH)
		{
			if (m_pDepthBuffer)
			{
				buffWidth = 480;
				buffHeight = 360;

				memcpy(m_pDepthBuffer, (uint16_t *)m_pDSNIRef->getZImage(), size_t(480 * 360 * sizeof(uint16_t)));

				return reinterpret_cast<void*>(m_pDepthBuffer);
			}
			else
				return NULL;
		}

		/*
		else
		{ // ONI_SENSOR_IR
			if (m_pFrameReader.infrared && m_pFrameBuffer.infrared)
			{
				buffWidth = 512;
				buffHeight = 424;

				IInfraredFrame* frame = NULL;
				HRESULT hr = m_pFrameReader.infrared->AcquireLatestFrame(&frame);
				if (SUCCEEDED(hr))
				{
					UINT16* data;
					UINT bufferSize;
					frame->AccessUnderlyingBuffer(&bufferSize, &data);
					memcpy(m_pFrameBuffer.infrared, data, 512*424*sizeof(UINT16));
				}
				if (frame)
				{
					frame->Release();
				}

				return reinterpret_cast<void*>(m_pFrameBuffer.infrared);
			}
		}*/
	}

	return NULL;
}

XN_THREAD_PROC DSNIStreamImpl::threadFunc(XN_THREAD_PARAM pThreadParam)
{
  DSNIStreamImpl* pStream = (DSNIStreamImpl*)pThreadParam;
  pStream->mainLoop();
  XN_THREAD_PROC_RETURN(XN_STATUS_OK);
}
