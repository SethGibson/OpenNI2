#include <string>
#include <sstream>
#include "DSAPIDriver.h"
#include "DSAPIDevice.h"
#include "DSAPI.h"
#include "DSAPIUtil.h"

#include <XnLog.h>

using namespace oni::driver;
using namespace dsapi_device;

static const char VENDOR_VAL[] = "TZYX";
static const char NAME_VAL[] = "DS4";

DSAPIDriver::DSAPIDriver(OniDriverServices* pDriverServices)
  : DriverBase(pDriverServices)
{

}
	
DSAPIDriver::~DSAPIDriver()
{}

OniStatus DSAPIDriver::initialize(DeviceConnectedCallback connectedCallback,
                                    DeviceDisconnectedCallback disconnectedCallback,
                                    DeviceStateChangedCallback deviceStateChangedCallback,
                                    void* pCookie)
{
	DriverBase::initialize(connectedCallback, disconnectedCallback, deviceStateChangedCallback, pCookie);

	/*
	IKinectSensor* pKinectSensor = NULL;
	hr = ::GetDefaultKinectSensor(&pKinectSensor);
	if (FAILED(hr))
	{
		if (pKinectSensor)
		{
			pKinectSensor->Release();
		}
		return ONI_STATUS_NO_DEVICE;
	}

	hr = pKinectSensor->Open();
	if (FAILED(hr))
	{
		pKinectSensor->Release();
		return ONI_STATUS_ERROR;
	}
	*/

	DSAPIRef cDSAPIRef = DSCreate(DS_DS4_PLATFORM);
	DSThird *cDSRgb = cDSAPIRef->accessThird();
	if (!cDSAPIRef->probeConfiguration())
		return ONI_STATUS_NO_DEVICE;
	if (!cDSRgb)
		return ONI_STATUS_NO_DEVICE;
	if (!cDSAPIRef->isCalibrationValid())
		return ONI_STATUS_ERROR;

	// Get sensor info
	OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
	
	//WCHAR sensorId[ONI_MAX_STR];
	//pKinectSensor->get_UniqueKinectId(ONI_MAX_STR, sensorId);
	
	uint32_t cSerialNo;
	//WCHAR sensorId[ONI_MAX_STR];
	cDSAPIRef->getCameraSerialNumber(cSerialNo);
	std::stringstream cStream;
	std::string cSerialStr = "";
	cStream << cSerialNo;
	cSerialStr = cStream.str();

	//size_t convertedChars = 0;
	//const size_t newsize = ONI_MAX_STR;
	//size_t origsize = wcslen(sensorId) + 1;
	//wcstombs_s(&convertedChars, pInfo->uri, origsize, sensorId, _TRUNCATE);
	
	xnOSStrCopy(pInfo->uri, cSerialStr.c_str(), ONI_MAX_STR);
	xnOSStrCopy(pInfo->vendor, VENDOR_VAL, ONI_MAX_STR);
	xnOSStrCopy(pInfo->name, NAME_VAL, ONI_MAX_STR);
	m_devices[pInfo] = NULL;
	deviceConnected(pInfo);
	deviceStateChanged(pInfo, S_OK); // Sensor is ready

	return ONI_STATUS_OK;
}

DeviceBase* DSAPIDriver::deviceOpen(const char* uri, const char* /*mode*/)
{
	for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (xnOSStrCmp(iter->Key()->uri, uri) == 0)
		{
			// Found
			if (iter->Value() != NULL)
			{
				// already using
				return iter->Value();
			}
			else
			{
				DSAPIRef cDSAPIRef = DSCreate(DS_DS4_PLATFORM);
				DSThird *cDSRgb = cDSAPIRef->accessThird();
				if (!cDSAPIRef->probeConfiguration())
					return NULL;
				if (!cDSRgb)
					return NULL;
				if (!cDSAPIRef->isCalibrationValid())
					return NULL;

				if (!cDSAPIRef->enableZ(true))
					return NULL;
				if (!cDSAPIRef->enableLeft(false))
					return NULL;
				if (!cDSAPIRef->enableRight(false))
					return NULL;
				if (!cDSAPIRef->setLRZResolutionMode(true,480,360,60,DS_LUMINANCE8))
					return NULL;
				if (!cDSAPIRef->enableLRCrop(true))
					return NULL;
				if (!cDSRgb->enableThird(true))
					return NULL;
				if (!cDSRgb->setThirdResolutionMode(true, 640, 480, 30, DS_RGB8))
					return NULL;
				if (!cDSAPIRef->startCapture())
					return NULL;
				DSAPIDevice* pDevice = XN_NEW(DSAPIDevice, cDSAPIRef, cDSRgb);
				iter->Value() = pDevice;
				return pDevice;
			}
		}
	}
	return NULL;	
}

void dsapi_device::DSAPIDriver::deviceClose(oni::driver::DeviceBase* pDevice)
{
	for (xnl::Hash<OniDeviceInfo*, oni::driver::DeviceBase*>::Iterator iter = m_devices.Begin(); iter != m_devices.End(); ++iter)
	{
		if (iter->Value() == pDevice)
		{
			iter->Value() = NULL;
			XN_DELETE(pDevice);
			return;
		}
	}

	// not our device?!
	XN_ASSERT(FALSE);
}

void DSAPIDriver::shutdown()
{}

OniStatus DSAPIDriver::tryDevice(const char* uri)
{
	return ONI_STATUS_OK;	
}

void* DSAPIDriver::enableFrameSync(StreamBase** pStreams, int streamCount)
{
	return NULL;
}

void DSAPIDriver::disableFrameSync(void* frameSyncGroup)
{}

ONI_EXPORT_DRIVER(dsapi_device::DSAPIDriver)