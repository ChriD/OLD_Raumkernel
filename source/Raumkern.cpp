#include "Raumkern.h"


namespace Raumkernel
{  

	Raumkern::Raumkern()
    {

		// create logging object and set default log level
        logger = new Logger();
		this->SetLogLevel(LogType::LOGINFO);

		// default sttings file path
		this->settingsXMLFilePath = SETTINGS_FILEPATH;
		
		// create zone and device manager and set logging object
		managerList.deviceManager = new DeviceManager();
		managerList.deviceManager->SetLoggerObject(logger);

		managerList.zoneManager = new ZoneManager();
		managerList.zoneManager->SetLoggerObject(logger);

		managerList.aliveCheckManager = new AliveCheckManager();
		managerList.aliveCheckManager->SetLoggerObject(logger);

		managerList.contentManager = new ContentManager();
		managerList.contentManager->SetLoggerObject(logger);

		// create object for the server
		raumkernServer = new RaumkernServer();
		raumkernServer->SetLoggerObject(logger);

		// set some default values
		hostRequestUrl = "";
		settingsXMLFilePath = "";
		networkAdapterName = "";

    }


	Raumkern::~Raumkern()
    {
		this->Log(LogType::LOGDEBUG, "Closing UPNP and destroying RAUMKERN Object", __FUNCTION__);
		
		// destroying raumkern has also to free memory used by zone and device manager, so do reset() which will cover this
		managerList.deviceManager->Reset();
		managerList.zoneManager->Reset();

		// delete raumkernServer (stop will be done in destructor)
		delete raumkernServer;

        //OpenHome::Net::UpnpLibrary::Close(); // TODO: Throws assertiaon error!!! I:ve no idea why!

		delete managerList.contentManager;
		delete managerList.deviceManager;
		delete managerList.zoneManager;
		delete managerList.aliveCheckManager;
        delete logger;		
    }


	// Raumkern::Log
	void Raumkern::Log(LogType _logType, std::string _log, std::string _location, bool _forceLog)
    {
		if (logger != nullptr)
			logger->Log(_logType, _log, _location, _forceLog);
    }


	// Raumkern::ApplicationLog
	void Raumkern::OnApplicationLog(ApplicationLogging::LogData _logData)
	{
		signalLog(_logData);
	}

	// Raumkern::SetLogLevel
	void Raumkern::SetLogLevel(LogType _logType)
	{
		if (logger != nullptr)
			logger->SetLogLevel(_logType);
	}


	// Raumkern::GetLogString
	// returns a  prebuilt log string to use for any type of log output
	std::string Raumkern::GetLogString(LogData _logData)
	{
		return ApplicationLogging::Logger::GetLogString(_logData);	
	}


	// Raumkern::Init
	bool Raumkern::Init()
    {       			
		logger->SubscribeSignalLog(typeSignalRaumkernLog::slot_type(boost::bind(&Raumkern::OnApplicationLog, this, _1)));
		managerList.deviceManager->SubscribeSignalConfigDeviceFound(typeSignalConfigDeviceFound::slot_type(boost::bind(&Raumkern::OnConfigDeviceFound, this, _1)));
		managerList.deviceManager->SubscribeSignalConfigDeviceLost(typeSignalConfigDeviceLost::slot_type(boost::bind(&Raumkern::OnConfigDeviceLost, this, _1)));
		managerList.aliveCheckManager->SubscribeSystemAliveStateChanged(typeSignalSystemAliveStateChanged::slot_type(boost::bind(&Raumkern::OnAliveStateChanged, this, _1)));
		
		// load settings RaumKernel needs to work from XML file
		if (!this->LoadRaumkernSettings())
		{
			this->Log(LogType::LOGCRITICAL, "Failed loading RaumKern settings! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}

		this->Log(LogType::LOGINFO, "Initializing RAUMKernel '" + RAUMKERNEL_VERSION + "'", __FUNCTION__, true);

		// ATTENTION: from now on the LogLevel got from settings XML file is used (if defined)			

		// start UPNP Control Point
		this->Log(LogType::LOGDEBUG, "Setting up UPNP...", __FUNCTION__);
		if (!this->InitUPNP())
		{
			this->Log(LogType::LOGCRITICAL, "Setting up UPNP Failed!", __FUNCTION__);
			return false;
		}
		this->Log(LogType::LOGDEBUG, "Setting up UPNP was done", __FUNCTION__);

        // start searching for Raumfeld  compopnents
		this->Log(LogType::LOGINFO, "Start searching for Raumfeld System Services and Media-Renderers. Please wait...", __FUNCTION__, true);
		managerList.deviceManager->StartFindDevices();
		
		// startung up raumkernServer if enabled
		if (raumkernSettings.RAUMKERNEL_SERVER_ENABLED)
		{
			this->Log(LogType::LOGDEBUG, "Starting RaumServer...", __FUNCTION__);
			if (!raumkernServer->StartServer())
				this->Log(LogType::LOGERROR, "RaumServer could not be started!");
		}
		

		// Init has done correct! Now it's UP to the UPNP discovering to find any Raumfeld Services
		// BE AWARE! Only that Init returns "true" does not mean that the System is ready for Operation. 
		return true;
    }


	// Raumkern::ResetRaumkern
	// this method should be called if the config device is lost and the systemState goes to "not Ready" or even if the ConfigDevice Appears again and Systen goest to state "Ready"
	void Raumkern::ResetRaumkern()
	{
		this->Log(LogType::LOGDEBUG, "Reseting Raumkern...", __FUNCTION__);

		if (managerList.zoneManager)
			managerList.zoneManager->Reset();
		if (managerList.deviceManager)
			managerList.deviceManager->Reset();
		
		hostRequestUrl = "";
	}


	// Raumkern::SetSettingsXMLFilePath
	void Raumkern::SetSettingsXMLFilePath(std::string _filePath)
	{
		settingsXMLFilePath = _filePath;
	}


	// Raumkern::LoadRaumkernSettings
	// load settings from XML application file. this method will be called very first at the Raumfeld::Init method
	bool Raumkern::LoadRaumkernSettings()
	{
		xml_document<> doc;
		xml_node<> *applicationNode, *raumKernNode, *settingsNode, *raumkernServerNode;


		if (settingsXMLFilePath.empty())
		{
			this->Log(LogType::LOGCRITICAL, "No settings file was defined! System will not be able to start!", __FUNCTION__);
			return false;
		}

		std::ifstream settingsFileStream(settingsXMLFilePath.c_str());
		if (settingsFileStream.fail())
		{
			this->Log(LogType::LOGCRITICAL, "Cannot open settings file '" + settingsXMLFilePath + "'", __FUNCTION__);
			return false;
		}


		std::vector<char> buffer((std::istreambuf_iterator<char>(settingsFileStream)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');
		// Parse the buffer using the xml file parsing library into doc
		doc.parse<0>(&buffer[0]);
		

		// find the root node which has to be the 'Application' node	
		applicationNode = doc.first_node("Application", 0, false);
		if (!applicationNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'Application' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		
		// inside the 'ApplicationNode' we have to find the 'Raumkern' node where the settings for our Kernel are defined
		raumKernNode = applicationNode->first_node("Raumkern", 0, false);
		if (!raumKernNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'Raumkern' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}

		// after we got settings for raumkern, first thing is that we do our log settings		
		this->logger->ClearLogAdapters();
		settingsNode = raumKernNode->first_node("LogAdapter", 0, false);
		while (settingsNode)
		{
			this->logger->AddLogAdapter(ApplicationLogging::Logger::GetLogAdapterTypeFromString(settingsNode->value()));
			settingsNode = settingsNode->next_sibling("LogAdapter", 0, false);
		}

		// get the log level as non obligatoric value. From now the LogLevel from the settings file will be used.	
		settingsNode = raumKernNode->first_node("LogLevel", 0, false);
		if (settingsNode)
		{
			this->SetLogLevel(this->logger->GetLogTypeFromString(settingsNode->value()));
		}


		// set up some standard values
		raumkernSettings.RAUMKERNEL_SERVER_TIMOUTREQUEST_SEC = 10;
		raumkernSettings.RAUMKERNEL_SERVER_TIMOUTCONTENT_SEC = 300;
		raumkernSettings.RAUMKERNEL_SERVER_FAILURERESTARTPOLLTIME = 4000;
		raumkernSettings.RAUMKERNEL_SERVER_PORT = "8080";
		raumkernSettings.RAUMKERNEL_SERVER_ENABLED = false;
		raumkernSettings.RAUMKERNEL_SERVER_MAXALLOWEDTHREADS = 10;


		// now go for the kernel settings. Some settings are obligate and we can not start the kernel if they are not filled in!
		// (maybe we may define some standard values here and only give some warning that node was not found on obligate settings?)
		settingsNode = raumKernNode->first_node("HostConfigDeviceName", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'HostConfigDeviceName' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		raumkernSettings.RAUMKERNEL_FRIENDLYNAME_CONFIGDEVICE = settingsNode->value();


		settingsNode = raumKernNode->first_node("MediaServerName", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'MediaServerName' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}		
		raumkernSettings.RAUMKERNEL_FRIENDLYNAME_MEDIASERVER = settingsNode->value();


		settingsNode = raumKernNode->first_node("HostRequestPort", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'HostRequestPort' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		raumkernSettings.RAUMKERNEL_REQUESTPORT = settingsNode->value();


		settingsNode = raumKernNode->first_node("MediaRendererIdentification", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'MediaRendererIdentification' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		raumkernSettings.RAUMKERNEL_MEDIARENDERER_IDENTIFICATION = settingsNode->value();


		settingsNode = raumKernNode->first_node("MediaServerIdentification", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'MediaServerIdentification' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		raumkernSettings.RAUMKERNEL_MEDIASERVER_IDENTIFICATION = settingsNode->value();


		settingsNode = raumKernNode->first_node("RaumfeldDescriptionVirtualMediaPlayer", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'RaumfeldDescriptionVirtualMediaPlayer' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		raumkernSettings.RAUNKERNEL_DESCRIPTIONVIRTUALMEDIAPLAYER = settingsNode->value();


		settingsNode = raumKernNode->first_node("RaumfeldManufacturer", 0, false);
		if (!settingsNode)
		{
			this->Log(LogType::LOGCRITICAL, "Settings file does not contain 'RaumfeldManufacturer' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			return false;
		}
		raumkernSettings.RAUMKERNEL_RAUMFELDMANUFACTURER = settingsNode->value();


		// look for a network adapter we should use. This is no mandatory setting. If not defined first usable network adapter will be used
		settingsNode = raumKernNode->first_node("NetworkAdapterName", 0, false);
		if (settingsNode)
		{
			this->SetNetworkAdapterName(settingsNode->value());
		}


		// getting some settings for the webServer
		raumkernServerNode = raumKernNode->first_node("Server", 0, false);
		if (raumkernServerNode)
		{
			settingsNode = raumkernServerNode->first_node("Enabled", 0, false);
			if (settingsNode)
			{
				std::string nodeValue = settingsNode->value();
				raumkernSettings.RAUMKERNEL_SERVER_ENABLED = nodeValue == "1" ? true : false;
			}

			settingsNode = raumkernServerNode->first_node("Port", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_SERVER_PORT = settingsNode->value();
			}

			settingsNode = raumkernServerNode->first_node("RequestPresetsFile", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_SERVER_REQUESTPRESETFILEPATH = settingsNode->value();
			}

			settingsNode = raumkernServerNode->first_node("TimeoutRequest", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_SERVER_TIMOUTREQUEST_SEC = std::stoi(settingsNode->value());
			}		

			settingsNode = raumkernServerNode->first_node("TimeoutContent", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_SERVER_TIMOUTCONTENT_SEC = std::stoi(settingsNode->value());
			}

			settingsNode = raumkernServerNode->first_node("MaxAllowedThreads", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_SERVER_MAXALLOWEDTHREADS = std::stoi(settingsNode->value());
			}

			settingsNode = raumkernServerNode->first_node("ServerFailureRestartPolltime", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_SERVER_FAILURERESTARTPOLLTIME = std::stoi(settingsNode->value());
			}


			
		}
		else
		{
			this->Log(LogType::LOGWARNING, "Settings file does not contain 'Server' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			raumkernSettings.RAUMKERNEL_SERVER_ENABLED = false;
		}


		// set up some standard values
		raumkernSettings.RAUMKERNEL_WEBCLIENT_ROOTPATH = "webclient/";
	
		// getting some settings for the webServer
		raumkernServerNode = raumKernNode->first_node("WebClient", 0, false);
		if (raumkernServerNode)
		{
			settingsNode = raumkernServerNode->first_node("rootPath", 0, false);
			if (settingsNode)
			{
				raumkernSettings.RAUMKERNEL_WEBCLIENT_ROOTPATH = settingsNode->value();			
			}			

		}
		else
		{
			this->Log(LogType::LOGWARNING, "Settings file does not contain 'WebClient' node! (" + settingsXMLFilePath + ")", __FUNCTION__);
			raumkernSettings.RAUMKERNEL_SERVER_ENABLED = false;
		}

	

		return true;
	}


	// Raumkern::SetNetworkAdapterName
	// define which network adapter to use for the UPNP Control Stack
	void Raumkern::SetNetworkAdapterName(std::string _networkAdapterName)
	{
		networkAdapterName = _networkAdapterName;
	}


	// Raumkern::GetLogger() 
	Logger* Raumkern::GetLogger()
	{
		return logger;
	}


	// Raumkern::GetZoneManager() 
	ZoneManager* Raumkern::GetZoneManager()
	{
		return managerList.zoneManager;
	}


	// Raumkern::GetDeviceManager() 
	DeviceManager* Raumkern::GetDeviceManager()
	{
		return managerList.deviceManager;
	}


	// Raumkern::GetRaumkernServer() 
	RaumkernServer* Raumkern::GetRaumkernServer()
	{
		return raumkernServer;
	}


	// Raumkern::GetContentManager() 
	ContentManager* Raumkern::GetContentManager()
	{
		return managerList.contentManager;
	}


	// Raumkern::InitUPNP
	// This one starts the UPNP client stack and returns true if there was no error starting the stack
	bool Raumkern::InitUPNP(void)
	{
		bool										ret = true;
		OpenHome::Net::InitialisationParams*		initParams;
		std::vector<OpenHome::NetworkAdapter*>*		subnetList;
		std::string									networkAdapterNameLocal, networkAdapterFullNameLocal;
		TIpAddress									subnet;
		boost::int16_t								subnetListIdx = -1;

		try
		{
			initParams = OpenHome::Net::InitialisationParams::Create();

			this->Log(LogType::LOGDEBUG, "Init UPNP Library", __FUNCTION__);

			//initParams->SetNetworkAdapterChangedListener(..) // TODO: @@@ Handle network adapter change?			

			OpenHome::Net::UpnpLibrary::Initialise(initParams);

			this->Log(LogType::LOGDEBUG, "Get network adapter list", __FUNCTION__);
			subnetList = OpenHome::Net::UpnpLibrary::CreateSubnetList();

			// find network adapterto use with the UPNP control stack
			if (networkAdapterName.empty())
			{
				this->Log(LogType::LOGWARNING, "No network device specified. Will use first one found!", __FUNCTION__);
				subnetListIdx = 0;
			}
			else
			{
				for (boost::uint16_t i = 0; i < (*subnetList).size(); i++)
				{
					networkAdapterNameLocal = (*subnetList)[i]->Name();
					if (networkAdapterNameLocal == networkAdapterName)
					{
						subnetListIdx = i;						
					}
					networkAdapterNameList.push_back(networkAdapterNameLocal);
				}
			}

			if (subnetListIdx < 0)
			{
				this->Log(LogType::LOGWARNING, "Network device '" + networkAdapterName + "' not found! Will use first one found!", __FUNCTION__);
				subnetListIdx = 0;
			}

			networkAdapterFullNameLocal = (*subnetList)[subnetListIdx]->FullName();
			networkAdapterNameLocal = (*subnetList)[subnetListIdx]->Name();			
			subnet = (*subnetList)[subnetListIdx]->Subnet();
			OpenHome::Net::UpnpLibrary::DestroySubnetList(subnetList);

			this->Log(LogType::LOGINFO, "Using NetworkAdapter: " + networkAdapterNameLocal, __FUNCTION__);

			this->Log(LogType::LOGDEBUG, "Starting UPNP ControlPoint", __FUNCTION__);

			OpenHome::Net::UpnpLibrary::StartCp(subnet);

		}
		catch (const std::exception& ex)
		{
			this->Log(LogType::LOGDEBUG, ex.what(), __FUNCTION__);
			ret = false;
		}
		catch (const std::string& ex)
		{
			this->Log(LogType::LOGDEBUG, ex, __FUNCTION__);
			ret = false;
		}
		catch (...)
		{
			this->Log(LogType::LOGDEBUG, "Unknown Error in Raumkern::InitUPNP", __FUNCTION__);
			ret = false;
		}

		return ret;
	}


	// Raumkern::ConfigDeviceFound
	// will be called if the RaumfeldConfig device appears on the network. 
	// we have to do some stuff for init to control the raumfeld system, then we are able to signal that System is ready
	void Raumkern::OnConfigDeviceFound(OpenHome::Net::CpDeviceCpp& _device)
	{
		std::string			location, zonesLongPollingId, devicesListLongPollingId;

		// reset Raumkernel to be sure we startup on a nice clean system
		// but only if its not the first init. On first init we do not have to do a reset!
		if (!hostRequestUrl.empty())
			this->ResetRaumkern();

		_device.GetAttribute("Upnp.Location", location);

		// build the host request url. This is the url all request (eg. for zone information,..) will go
		// location uri looks like this "http://10.0.0.5:43893/312d44ff-67c0-4a93-96db-b1b53733bc6a.xml"		
		hostRequestUrl = location.substr(0, location.find_last_of(":")) + ":" + raumkernSettings.RAUMKERNEL_REQUESTPORT + "/" + REQUESTPATH + "/";
		this->Log(LogType::LOGDEBUG, "RequestUrl built: " + hostRequestUrl, __FUNCTION__);

		// the zone and device manager has to do some request so set the host request url
		managerList.zoneManager->SetRequestUrl(hostRequestUrl);
		managerList.deviceManager->SetRequestUrl(hostRequestUrl);
		managerList.aliveCheckManager->SetRequestUrl(hostRequestUrl);
		managerList.contentManager->SetRequestUrl(hostRequestUrl);

		// do 'sync' read of zones. This will call our parse method and signals out that zone configuration was changed
		this->Log(LogType::LOGINFO, "Requesting list of zones present on the Host", __FUNCTION__);
		zonesLongPollingId = managerList.zoneManager->RequestZones();

		//this->Log(LogType::LOGINFO, "Requesting list of devices seen by the Host", __FUNCTION__);
		//devicesListLongPollingId = deviceManager->RequestDevices();

		// After getting the zones and devices 'in sync' we do set up a thread which does 'long polling'. and will parse and signal out each time the zone configuration or device list has changed
		managerList.zoneManager->StartRequestZonesWorkerThread(zonesLongPollingId);
		//deviceManager->StartRequestDevicesWorkerThread(devicesListLongPollingId);
		managerList.aliveCheckManager->StartAliveCheck();

		this->Log(LogType::LOGINFO, "Raumkern is ready for duty!", __FUNCTION__, true);

		signalSystemReady();
	}


	// Raumkern::ConfigDeviceLost
	// will be called if the RaumfeldConfig device disappears on the network. 
	// we have to signal that system is now "not ready"
	void Raumkern::OnConfigDeviceLost(OpenHome::Net::CpDeviceCpp& _device)
	{
		this->ResetRaumkern();
		signalSystemShutdown();
	}

	// Raumkern::GetAvailableNetworkAdapterNames
	// returns the available network adapter names the UPNP framework has found
	// will be populated after Init() in InitUPNP()
	std::vector<std::string> Raumkern::GetAvailableNetworkAdapterNames()
	{
		return networkAdapterNameList;
	}


	// Raumkern::OnAliveStateChanged
	// will be called if the RaumfeldSystem does not PONG anymore
	// we have to signal that system is now "not ready"
	void Raumkern::OnAliveStateChanged(bool _isAlive)
	{
		if (!_isAlive)
		{
			this->ResetRaumkern();
			signalSystemShutdown();
		}
	}

}
