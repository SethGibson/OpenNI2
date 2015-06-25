#include <string>
#include <sstream>
#include "DSNIDriver.h"
#include "DSNIDevice.h"
#include "DSAPI.h"
#include "DSAPIUtil.h"

#include <XnLog.h>

using namespace oni::driver;
using namespace dsni_device;

static const char VENDOR_VAL[] = "TZYX";
static const char NAME_VAL[] = "DS4";

DSNIDriver::DSNIDriver(OniDriverServices* pDriverServices)
  : DriverBase(pDriverServices)
{

}
	
DSNIDriver::~DSNIDriver()
{}

OniStatus DSNIDriver::initialize(DeviceConnectedCallback connectedCallback,
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

	DSAPIRef cDSNIRef = DSCreate(DS_DS4_PLATFORM);
	DSThird *cDSRgb = cDSNIRef->accessThird();
	if (!cDSNIRef->probeConfiguration())
		return ONI_STATUS_NO_DEVICE;
	if (!cDSRgb)
		return ONI_STATUS_NO_DEVICE;
	if (!cDSNIRef->isCalibrationValid())
		return ONI_STATUS_ERROR;

	// Get sensor info
	OniDeviceInfo* pInfo = XN_NEW(OniDeviceInfo);
	
	//WCHAR sensorId[ONI_MAX_STR];
	//pKinectSensor->get_UniqueKinectId(ONI_MAX_STR, sensorId);
	
	uint32_t cSerialNo;
	//WCHAR sensorId[ONI_MAX_STR];
	cDSNIRef->getCameraSerialNumber(cSerialNo);
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

DeviceBase* DSNIDriver::deviceOpen(const char* uri, const char* /*mode*/)
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
				DSAPIRef cDSNIRef = DSCreate(DS_DS4_PLATFORM);
				DSThird *cDSRgb = cDSNIRef->accessThird();
				if (!cDSNIRef->probeConfiguration())
					return NULL;
				if (!cDSRgb)
					return NULL;
				if (!cDSNIRef->isCalibrationValid())
					return NULL;

				if (!cDSNIRef->enableZ(true))
					return NULL;
				if (!cDSNIRef->enableLeft(false))
					return NULL;
				if (!cDSNIRef->enableRight(false))
					return NULL;
				if (!cDSNIRef->setLRZResolutionMode(true,480,360,60,DS_LUMINANCE8))
					return NULL;
				if (!cDSNIRef->enableLRCrop(true))
					return NULL;
				if (!cDSRgb->enableThird(true))
					return NULL;
				if (!cDSRgb->setThirdResolutionMode(true, 640, 480, 30, DS_RGB8))
					return NULL;
				if (!cDSNIRef->startCapture())
					return NULL;
				DSNIDevice* pDevice = XN_NEW(DSNIDevice, cDSNIRef, cDSRgb);
				iter->Value() = pDevice;
				return pDevice;
			}
		}
	}
	return NULL;	
}

void dsni_device::DSNIDriver::deviceClose(oni::driver::DeviceBase* pDevice)
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

void DSNIDriver::shutdown()
{}

OniStatus DSNIDriver::tryDevice(const char* uri)
{
	return ONI_STATUS_OK;	
}

void* DSNIDriver::enableFrameSync(StreamBase** pStreams, int streamCount)
{
	return NULL;
}

void DSNIDriver::disableFrameSync(void* frameSyncGroup)
{}

ONI_EXPORT_DRIVER(dsni_device::DSNIDriver)