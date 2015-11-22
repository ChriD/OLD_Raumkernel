#include "RaumkernObject.h"


namespace Raumkernel
{

	ManagerList		managerList;


	RaumkernObject::RaumkernObject()
	{
		//managerList = nullptr;
		logger = nullptr;
	}

	RaumkernObject::~RaumkernObject()
	{

	}

	// RaumkernObject::SetLoggerObject
	void RaumkernObject::SetLoggerObject(Logger *_logger)
	{
		this->logger = _logger;
	}

	// RaumkernObject::GetLoggerObject
	Logger* RaumkernObject::GetLoggerObject()
	{	
		return this->logger;
	}

	// RaumkernObject::Log
	void RaumkernObject::Log(LogType _logType, std::string _log, std::string _location, bool _forceLog)
	{
		if (logger != nullptr)
			logger->Log(_logType, _log, _location, _forceLog);
	}

	void RaumkernObject::Sleep(boost::uint32_t _sleepMS)
	{
		// boost sleep does segfault on linux! so we use c++11 sleep
		//boost::this_thread::sleep(boost::posix_time::milliseconds(_sleepMS));+
		std::this_thread::sleep_for(std::chrono::microseconds(_sleepMS * 1000));
	}

	boost::int32_t RaumkernObject::toInt(std::string _string)
	{
		try
		{
			return std::stoi(_string);
		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGWARNING, "Exception on stoi: " + _string + " Exception: "  + ex.what(), __FUNCTION__);
		}
		return 0;
	}

	
	DeviceManager* RaumkernObject::GetDeviceManager()
	{
		return managerList.deviceManager;
	}

	ZoneManager* RaumkernObject::GetZoneManager()
	{
		return managerList.zoneManager;
	}
	
	

	UPNPDevice::UPNPDevice() : RaumkernObject()
	{
		cpDevice = nullptr;
		UDN = "";
		deviceType = "";
		friendlyName = "";
		modelName = "";
		modelDescription = "";
		manufacturer = "";
		manufacturerUrl = "";
		deviceXML = "";
		modelNumber = "";
		serialNumber = "";
	}

	UPNPDevice::~UPNPDevice()
	{
		// no need to remove subscriptions! delete of proxies will handle this correct!
		this->DeleteProxies();
	}

	void UPNPDevice::SetUDN(std::string _UDN)
	{
		this->UDN = _UDN;
	}

	void UPNPDevice::SetDeviceType(std::string _deviceType)
	{
		this->deviceType = _deviceType;
	}

	void UPNPDevice::SetFriendlyName(std::string _friendlyName)
	{
		this->friendlyName = _friendlyName;
	}

	void UPNPDevice::SetModelName(std::string _modelName)
	{
		this->modelName = _modelName;
	}

	void UPNPDevice::SetModelDescription(std::string _modelDescription)
	{
		this->modelDescription = _modelDescription;
	}

	void UPNPDevice::SetManufacturer(std::string _manufacturer)
	{
		this->manufacturer = _manufacturer;
	}

	void UPNPDevice::SetManufacturerUrl(std::string _manufacturerUrl)
	{
		this->manufacturerUrl = _manufacturerUrl;
	}

	void UPNPDevice::SetDeviceXML(std::string _deviceXML)
	{
		this->deviceXML = _deviceXML;
	}

	void UPNPDevice::SetModelNumber(std::string _modelNumber)
	{
		this->modelNumber = _modelNumber;
	}

	void UPNPDevice::SetSerialNumber(std::string _serialNumber)
	{
		this->serialNumber = _serialNumber;
	}

	std::string UPNPDevice::GetUDN()
	{
		return this->UDN;
	}

	std::string UPNPDevice::GetDeviceType()
	{
		return this->deviceType;
	}

	std::string UPNPDevice::GetFriendlyName()
	{
		return this->friendlyName;
	}

	std::string UPNPDevice::GetModelName()
	{
		return this->modelName;
	}

	std::string UPNPDevice::GetModelDescription()
	{
		return this->modelDescription;
	}

	std::string UPNPDevice::GetManufacturer()
	{
		return this->manufacturer;
	}

	std::string UPNPDevice::GetManufacturerUrl()
	{
		return this->manufacturerUrl;
	}

	std::string UPNPDevice::GetDeviceXML()
	{
		return this->deviceXML;
	}

	std::string UPNPDevice::GetModelNumber()
	{
		return this->modelNumber;
	}

	std::string UPNPDevice::GetSerialNumber()
	{
		return this->serialNumber;
	}


	std::string UPNPDevice::GetIdentificationString()
	{
		// friendly name is not updated if zone config changed!
		//return this->friendlyName;
		return this->UDN;
	}


	void UPNPDevice::SetCpDevice(CpDeviceCpp *_cpDevice)
	{
		//if (this->cpDevice)
		//	this->RemoveSubscriptions();
		this->cpDevice = _cpDevice;
		if (this->cpDevice)
		{
			this->CreateProxies(true);
			this->AddSubscriptions();
		}
		else
			this->CPDeviceLost();
	}

	void UPNPDevice::CPDeviceLost()
	{
		this->DeleteProxies();
	}

	void UPNPDevice::CreateProxies(bool _allowCreate)
	{
		// overload this method to create the proxies
	}

	void UPNPDevice::DeleteProxies()
	{
		// overload this method to delete the created proxies in "CreateProxies"
	}

	void UPNPDevice::AddSubscriptions()
	{
		// overload this method to add subscriptions to upnp services
	}

	void UPNPDevice::RemoveSubscriptions()
	{
		// overload this method to add subscriptions to upnp services
	}


	void UPNPDevice::UPNPException(OpenHome::Exception _exception, std::string _location)
	{
		this->Log(LogType::LOGERROR, "UPNP Exception was thrown!", _location);
		this->Log(LogType::LOGERROR, _exception.Message(), _location);
	}

}