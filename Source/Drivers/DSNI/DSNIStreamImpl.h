#ifndef _DSNI_STREAM_IMPL_H_
#define _DSNI_STREAM_IMPL_H_

#include <memory>
#include "BaseDSNIStream.h"
#include "XnList.h"
#include "DSAPI.h"
#include "DSAPIUtil.h"

namespace dsni_device
{
	typedef DSAPI* DSAPIRef;
	class DSNIStreamImpl
	{
	public:
		DSNIStreamImpl(DSAPIRef pDSNIRef, DSThird *pDSThird, OniSensorType sensorType, LONGLONG basePerfCounter);

		virtual ~DSNIStreamImpl();

		void addStream(BaseDSNIStream* stream);
		void removeStream(BaseDSNIStream* stream);
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

		const DSAPIRef& getDSNIRef() { return m_pDSNIRef;  }
		const DSCalibIntrinsicsRectified& getZIntrinsicsRect() { return m_pZIntrins; }
		const DSCalibIntrinsicsRectified& getRgbIntrinsicsRect() { return m_pRgbIntrins; }
		const double* getZToThirdTrans() { return m_pZtoThirdTrans; }


	private:
		void setDefaultVideoMode();
		void createFrameBuffer();
		void destroyFrameBuffer();
		void* populateFrameBuffer(int& buffWidth, int& buffHeight);

		static XN_THREAD_PROC threadFunc(XN_THREAD_PARAM pThreadParam);

	private:
		DSAPIRef m_pDSNIRef;
		DSThird *m_pDSThird;
		DSCalibIntrinsicsRectified m_pZIntrins;
		DSCalibIntrinsicsRectified m_pRgbIntrins;
		double m_pZtoThirdTrans[3];
		
		uint8_t* m_pColorBuffer;
		uint16_t* m_pDepthBuffer;

		/*
		union
		{
			uint8_t* color;
			uint16_t* depth;
			//uint16_t* infrared;
		} m_pFrameBuffer;*/


		OniSensorType m_sensorType;
		OniImageRegistrationMode m_imageRegistrationMode;
		OniVideoMode m_videoMode;
		xnl::List<BaseDSNIStream*> m_streamList;

		// Thread
		bool m_running;
		LONGLONG m_perfCounter;
		double m_perfFreq;
		XN_THREAD_HANDLE m_threadHandle;
	};
}
#endif
