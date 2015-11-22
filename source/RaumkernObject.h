#ifndef RAUMKERNOBJECT_H
#define RAUMKERNOBJECT_H


#include "os.h"

#include <chrono>
#include <thread>
#include <string> 

#include <OpenHome/Net/Cpp/OhNet.h>
#include <OpenHome/Net/Cpp/CpDevice.h>
#include <OpenHome/Net/Cpp/CpDeviceDv.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include <boost/cstdint.hpp>
#include <boost/thread.hpp>

#include "Logger.h"
#include "Global.h"
#include "ManagerList.h" 


using namespace ApplicationLogging;
using namespace OpenHome::Net;


namespace Raumkernel
{	

	class DeviceManager;
	class ZoneManager;

	class RaumkernObject
	{
		public:
			RaumkernObject();
			virtual ~RaumkernObject();
			void SetLoggerObject(Logger *_logger);
			Logger* GetLoggerObject();
					
			DeviceManager* GetDeviceManager();
			ZoneManager* GetZoneManager();

		protected:
			void Log(LogType _logType, std::string _log, std::string _loaction = "", bool _forceLog = false);
			void Sleep(boost::uint32_t _sleepMS);

			boost::int32_t toInt(std::string _string);

		private:
			Logger	*logger;
	};	


	class UPNPDevice : public RaumkernObject
	{
		public:
			UPNPDevice();
			virtual ~UPNPDevice();	

			void SetUDN(std::string _UDN);
			void SetDeviceType(std::string _deviceType);
			void SetFriendlyName(std::string _friendlyName);
			void SetModelName(std::string _modelName);
			void SetModelDescription(std::string _modelDescription);
			void SetManufacturer(std::string _manufacturer);
			void SetManufacturerUrl(std::string _manufacturerUrl);
			void SetDeviceXML(std::string _deviceXML);
			void SetModelNumber(std::string _modelNumber);
			void SetSerialNumber(std::string _serialNumber);

			std::string GetUDN();
			std::string GetDeviceType();
			std::string GetFriendlyName();
			std::string GetModelName();
			std::string GetModelDescription();
			std::string GetManufacturer();
			std::string GetManufacturerUrl();
			std::string GetDeviceXML();
			std::string GetModelNumber();
			std::string GetSerialNumber();

			std::string GetIdentificationString();

			void SetCpDevice(CpDeviceCpp *_cpDevice);		

		protected:
			std::string UDN;
			std::string deviceType;
			std::string friendlyName;
			std::string modelName;
			std::string modelDescription;
			std::string manufacturer;
			std::string manufacturerUrl;
			std::string deviceXML;
			std::string modelNumber;
			std::string serialNumber;

			CpDeviceCpp *cpDevice;

			virtual void AddSubscriptions();
			virtual void RemoveSubscriptions();

			virtual void CreateProxies(bool _allowCreate = true);
			virtual void DeleteProxies();

			virtual void CPDeviceLost();

			virtual void UPNPException(OpenHome::Exception _exception, std::string _location);

		private:
		
	};

	extern ManagerList managerList;

}

#endif // RAUMKERNOBJECT_H