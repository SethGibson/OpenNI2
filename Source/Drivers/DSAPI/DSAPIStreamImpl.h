#ifndef _DSAPI_STREAM_IMPL_H_
#define _DSAPI_STREAM_IMPL_H_

#include <memory>
#include "BaseDSAPIStream.h"
#include "XnList.h"
#include "DSAPI.h"
#include "DSAPIUtil.h"
/*
struct IKinectSensor;
struct ICoordinateMapper;
struct IFrameDescription;
struct IColorFrameReader;
struct IDepthFrameReader;
struct IInfraredFrameReader;
*/

namespace dsapi_device
{
	//typedef std::shared_ptr<DSAPI> DSAPIRef;
	typedef DSAPI* DSAPIRef;
	class DSAPIStreamImpl
	{
	public:
		DSAPIStreamImpl(DSAPIRef pDSAPIRef, DSThird *pDSThird, OniSensorType sensorType, LONGLONG basePerfCounter);

		virtual ~DSAPIStreamImpl();

		void addStream(BaseDSAPIStream* stream);
		void removeStream(BaseDSAPIStream* stream);
		unsigned int getStreamCount();

		void setVideoMode(OniVideoMode* videoMode);

		OniStatus virtual start();
		void virtual stop();
		bool  isRunning() { return m_running; }

		OniSensorType getSensorType () { return m_sensorType; }
		void setSensorType(OniSensorType sensorType);

		void mainLoop();

		XnDouble getHorizontalFov();
		XnDouble getVerticalFov();

		OniImageRegistrationMode getImageRegistrationMode() const { return m_imageRegistrationMode; }
		void setImageRegistrationMode(OniImageRegistrationMode mode) { m_imageRegistrationMode = mode; }

		const DSAPIRef& getDSAPIRef() { return m_pDSAPIRef;  }
		const DSCalibIntrinsicsRectified& getZIntrinsicsRect() { return m_pZIntrins; }
		const DSCalibIntrinsicsRectified& getRgbIntrinsicsRect() { return m_pRgbIntrins; }
		const double* getZToThirdTrans() { return m_pZtoThirdTrans; }


	private:
		void setDefaultVideoMode();
		
		// frame description is width height pixelformat etc ?
		//IFrameDescription* getFrameDescription(OniSensorType sensorType);
		
		void createFrameBuffer();
		void destroyFrameBuffer();
		//void openFrameReader();
		//void closeFrameReader();
		void* populateFrameBuffer(int& buffWidth, int& buffHeight);

		static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam);

	private:
		DSAPIRef m_pDSAPIRef;
		DSThird *m_pDSThird;
		DSCalibIntrinsicsRectified m_pZIntrins;
		DSCalibIntrinsicsRectified m_pRgbIntrins;
		double m_pZtoThirdTrans[3];
		
		union
		{
			uint8_t* color;
			uint16_t* depth;
			//uint16_t* infrared;
		} m_pFrameBuffer;

		/*
		union {
		IColorFrameReader* color;
		IDepthFrameReader* depth;
		IInfraredFrameReader* infrared;
		} m_pFrameReader;*/

		OniSensorType m_sensorType;
		OniImageRegistrationMode m_imageRegistrationMode;
		OniVideoMode m_videoMode;
		xnl::List<BaseDSAPIStream*> m_streamList;

		// Thread
		bool m_running;
		LONGLONG m_perfCounter;
		double m_perfFreq;
		XN_THREAD_HANDLE m_threadHandle;
	};
}
#endif
