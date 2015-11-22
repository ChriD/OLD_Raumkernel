#include "DeviceManager.h"
#include "Renderer.h"
#include "MediaServer.h"
#include "ContentManager.h"

namespace Raumkernel
{
	

		DeviceManager::DeviceManager() : HTTPRequestManager()
		{
		}

		DeviceManager::~DeviceManager()
		{
		}

		//DeviceManager::GetLastUpdateId
		boost::uint32_t DeviceManager::GetLastUpdateIdRendererState()
		{
			boost::mutex::scoped_lock scoped_lock(mutexUpdateIdRendererState);
			return lastUpdateIdRendererState;
		}

		//DeviceManager::GetLastUpdateId
		void DeviceManager::SetLastUpdateIdRendererState(boost::uint32_t _lastUpdateIdRendererState)
		{
			boost::mutex::scoped_lock scoped_lock(mutexUpdateIdRendererState);
			lastUpdateIdRendererState = _lastUpdateIdRendererState;
		}


		// OBSOLETE METHOD!!!!
		// DeviceManager::StartRequestDevicesWorkerThread
		void DeviceManager::StartRequestDevicesWorkerThread(std::string _longPollingUpdateId)
		{
			this->Log(LogType::LOGDEBUG, "Starting worker thread for device requests...", __FUNCTION__);
			// becasue we can not kill the thread proper on reseting the kernel we cant do this check here!
			//if (!requestDevicesThread.joinable())
				requestDevicesThread = boost::thread(&DeviceManager::RequestDevicesWorkerThread, this, _longPollingUpdateId);
			//else
			//	this->Log(LogType::LOGDEBUG, "Worker thread already running!", __FUNCTION__);
		}


		// OBSOLETE METHOD!!!!
		// DeviceManager::RequestDevicesWorkerThread
		// worker thread for polling and long polling the device list of the system
		void DeviceManager::RequestDevicesWorkerThread(std::string _longPollingUpdateId)
		{
			std::string longPollingUpdateId = _longPollingUpdateId;
		
			do
			{
				longPollingUpdateId = this->RequestDevices(longPollingUpdateId);									
			} 
			while (longPollingUpdateId != LONGPOLLINGREQUESTQUIT);
				

			this->Log(LogType::LOGDEBUG, "RequestDevicesWorkerThread finished", __FUNCTION__);
		}
		

		// DeviceManager::SubscribeSignalZoneConfigurationChanged
		void DeviceManager::SubscribeSignalDeviceListChanged(const typeSignalDeviceListChanged::slot_type &_subscriber)
		{
			signalDeviceListChanged.connect(_subscriber);
		}


		// DeviceManager::SubscribeSignalConfigDeviceFound
		void DeviceManager::SubscribeSignalConfigDeviceFound(const typeSignalConfigDeviceFound::slot_type &_subscriber)
		{
			signalConfigDeviceFound.connect(_subscriber);
		}


		// DeviceManager::SubscribeSignalConfigDeviceLost
		void DeviceManager::SubscribeSignalConfigDeviceLost(const typeSignalConfigDeviceLost::slot_type &_subscriber)
		{
			signalConfigDeviceLost.connect(_subscriber);
		}


		// DeviceManager::SubscribeSignalMediaRendererStateChanged
		void DeviceManager::SubscribeSignalMediaRendererStateChanged(const typeSignalMediaRendererStateChanged::slot_type &_subscriber)
		{
			signalMediaRendererStateChanged.connect(_subscriber);
		}

		// DeviceManager::SubscribeSignalMediaServerFound
		void DeviceManager::SubscribeSignalMediaServerFound(const typeSignalMediaServerFound::slot_type &_subscriber)
		{
			signalMediaServerFound.connect(_subscriber);
		}

		// DeviceManager::SubscribeSignalMediaServerLost
		void DeviceManager::SubscribeSignalMediaServerLost(const typeSignalMediaServerLost::slot_type &_subscriber)
		{
			signalMediaServerLost.connect(_subscriber);
		}


		// DeviceManager::OnMediaRendererStateChanged
		void DeviceManager::OnMediaRendererStateChanged(std::string _rendererUDN)
		{
			signalMediaRendererStateChanged(_rendererUDN);
		}


		// DeviceManager::ClearLists
		void DeviceManager::ClearLists()
		{

			this->ClearMediaRendererMap();
		    // do not clear device map! device map has to be peristent!
			//this->ClearUPNPDeviceMap();		
			this->ClearMediaServerMap();

			signalDeviceListChanged();
		}

		// DeviceManager::ClearMediaRendererMaps
		void DeviceManager::ClearMediaRendererMap()
		{


			// clear reseved space of renderer classes, 
			for (auto &mapItem : mediaRendererVirtualMap)
			{
				MediaRenderer *renderer = mapItem.second;
				delete renderer;
			}

			for (auto &mapItem : mediaRendererMap)
			{
				MediaRenderer *renderer = mapItem.second;
				delete renderer;
			}

			// then we can clear the lists containing the pointer sto this allocated memory
			mediaRendererVirtualMap.clear();
			mediaRendererMap.clear();
		}

		// DeviceManager::ClearMediaServerMap
		void DeviceManager::ClearMediaServerMap()
		{
			// clear reseved space of renderer classes, 
			for (auto &mapItem : mediaServerMap)
			{
				MediaServer *mediaServer = mapItem.second;
				delete mediaServer;
			}
			
			// then we can clear the lists containing the pointer to this allocated memory			
			mediaServerMap.clear();
		}

		// DeviceManager::ClearUPNPDeviceMap
		void DeviceManager::ClearUPNPDeviceMap()
		{
			for (auto &mapItem : upnpDeviceMap)
			{
				OpenHome::Net::CpDeviceCpp* device = mapItem.second;
				device->RemoveRef();
			}

			upnpDeviceMap.clear();
		}


		// DeviceManager::Reset
		void DeviceManager::Reset()
		{
			this->Log(LogType::LOGDEBUG, "Reseting Device Manager...", __FUNCTION__);

			this->Log(LogType::LOGDEBUG, "Waiting for threads to finish...", __FUNCTION__);
			if (requestDevicesThread.joinable())
			{				
				//requestDevicesThread.interrupt(); 
				// ATTENTION: Due the fact we are using cpp-netlib, the interrupt won't work so would would stay stick on jon
				// for now we have to live with this issue!
				//requestDevicesThread.join();
			}

			boost::mutex::scoped_lock scoped_lock(mutexRendererServerListUpdate);
			
			this->ClearLists();		
			this->Log(LogType::LOGDEBUG, "Reset Done...", __FUNCTION__);
		}

		// DeviceManager::IsRendererServerListLockedForUpdate		
		bool DeviceManager::IsRendererServerListLockedForUpdate()
		{
			if (mutexRendererServerListUpdate.try_lock())
			{
				mutexRendererServerListUpdate.unlock();
				return false;
			}
			else
				return true;
		}

		// DeviceManager::WaitTillRendererServerListIsReady		
		void DeviceManager::WaitTillRendererServerListIsReady()
		{
			while (this->IsRendererServerListLockedForUpdate())
			{
				this->Sleep(10);
			}
		}

		// DeviceManager::LockRendererServerListForUpdate	
		void DeviceManager::LockRendererServerListForUpdate(bool _lock)
		{
			if (_lock)
			{
				mutexRendererServerListUpdate.lock();
			}
			else
			{
				mutexRendererServerListUpdate.unlock();
			}			
		}

		// DeviceManager::Lock	
		void DeviceManager::Lock()
		{
			LockRendererServerListForUpdate(true);
			mutexUPNPDeviceList.lock();
		}

		// DeviceManager::UnLock	
		void DeviceManager::UnLock()
		{
			LockRendererServerListForUpdate(false);
			mutexUPNPDeviceList.unlock();
		}


		// OBSOLETE METHOD!!!!
		// DeviceManager::ParseDevices		
		void DeviceManager::ParseDevices(std::string _devicesXML)
		{									
			
			//boost::mutex::scoped_lock scoped_lock(mutexRendererServerListUpdate);
			this->Lock();

			try
			{


				xml_document<> doc;
				xml_node<> *devicesNode, *deviceNode;
				std::string	deviceLocation, deviceType, deviceUDN, deviceXML;
				MediaRenderer* mediaRenderer;
				MediaServer *mediaServer;

				this->Log(LogType::LOGDEBUG, "Devices list changed! Parsing new data", __FUNCTION__);

				// we have to clear the maps holding the renderers because we got new one or some dissapeard. so we startup clean!
				this->ClearMediaRendererMap();
				this->ClearMediaServerMap();

				// ATTENTION: ParseDevices may be called before the UPNP control point has found the device (and added in 'upnpDeviceList') 
				// so we may not call any stuff on the device itself. we only have the info of the given xml here!
				// in the XML there is information of the device xml. There all the information is, we do need!

				// to parse the string we have to put it ino char* (because c_str() returns const char*)
				char* cstr = new char[_devicesXML.size() + 1];
				strcpy(cstr, _devicesXML.c_str());
				doc.parse<0>(cstr);

				// find the root node which has to be the 'devices' node	
				devicesNode = doc.first_node("devices", 0, false);
				if (!devicesNode)
				{
					//this->Log(LogType::LOGERROR, "Requested devices XML from host does not contain device information!", __FUNCTION__);
					throw std::runtime_error("Requested devices XML from host does not contain device information!");
				}

				// run through device nodes
				deviceNode = devicesNode->first_node("device", 0, false);
				while (deviceNode)
				{
					// here we are on a 'device' node which may be any UPNP device, we only store devices we may handle, so get 'type' and store rendreres in renderer lists

					deviceType = deviceNode->first_attribute("type", 0, false)->value();
					deviceLocation = deviceNode->first_attribute("location", 0, false)->value();
					deviceUDN = deviceNode->first_attribute("udn", 0, false)->value();
					deviceUDN = Utils::FormatUDN(deviceUDN);

					this->Log(LogType::LOGDEBUG, "Device node found in XML (" + deviceUDN + ")", __FUNCTION__);

					// check if device is of type mediaRenderer (those of this type we can use)
					if (deviceType.find(raumkernSettings.RAUMKERNEL_MEDIARENDERER_IDENTIFICATION) != std::string::npos)
					{
						deviceXML = this->RequestDeviceDetailXML(deviceLocation);
						if (!deviceXML.empty())
						{
							this->Log(LogType::LOGDEBUG, "Create Object for Device (" + deviceUDN + ")", __FUNCTION__);
							mediaRenderer = this->CreateMediaRendererObjectFromDeviceXML(deviceXML);
							if (mediaRenderer)
							{
								if (mediaRenderer->IsVirtualRenderer())
									mediaRendererVirtualMap.insert(std::make_pair(mediaRenderer->GetUDN(), mediaRenderer));
								else
									mediaRendererMap.insert(std::make_pair(mediaRenderer->GetUDN(), mediaRenderer));
							}
							else
							{
								this->Log(LogType::LOGERROR, "Object for device '" + deviceType + "' with UDN: '" + deviceUDN + "' can not be created", __FUNCTION__);
							}
						}
						else
						{
							this->Log(LogType::LOGWARNING, "Cant not get device XML from '" + deviceType + "' with UDN: '" + deviceUDN + "'", __FUNCTION__);
						}
					}

					// check if device is of type mediaServer
					// in fact there can only be one raumfeld media server we can use
					if (deviceType.find(raumkernSettings.RAUMKERNEL_MEDIASERVER_IDENTIFICATION) != std::string::npos)
					{
						deviceXML = this->RequestDeviceDetailXML(deviceLocation);
						if (!deviceXML.empty())
						{
							mediaServer = this->CreateMediaServerObjectFromDeviceXML(deviceXML);
							if (mediaServer)
							{
								if (mediaServer->IsRaumfeldMediaServer())
									mediaServerMap.insert(std::make_pair(mediaServer->GetUDN(), mediaServer));
								else
								{
									// we can not do anything with a media server which is no raumfeld media server, so....
									// maybee in future we can us it in any way... But for now....
									delete mediaServer;
								}
							}
							else
							{
								this->Log(LogType::LOGERROR, "Object for device '" + deviceType + "' with UDN: '" + deviceUDN + "' can not be created", __FUNCTION__);
							}
						}
						else
						{
							this->Log(LogType::LOGWARNING, "Cant not get device XML from '" + deviceType + "' with UDN: '" + deviceUDN + "'", __FUNCTION__);
						}
					}

					deviceNode = deviceNode->next_sibling("device", 0, false);
				}


				delete[] cstr;

			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
			catch (const std::string& ex)
			{
				this->Log(LogType::LOGERROR, ex, __FUNCTION__);
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "Unresolved Error", __FUNCTION__);
			}


			this->UnLock();
			
			signalDeviceListChanged();
		}


		// DeviceManager::SetValuesUPNPDevice
		void DeviceManager::SetValuesUPNPDevice(UPNPDevice* _upnpDevice, xml_node<> *_deviceNode)
		{
			xml_document<> doc;
			xml_node<> *deviceNode = _deviceNode, *valueNode;
			std::string	deviceType, deviceUDN, deviceFriendlyName, deviceModelDescription, deviceModelName, deviceManufacturer, deviceManufacturerUrl;
			std::string	deviceModelNumber, deviceSerialNumber;

			deviceType = deviceNode->first_node("deviceType", 0, false)->value();
			deviceUDN = deviceNode->first_node("UDN", 0, false)->value();
			deviceUDN = Utils::FormatUDN(deviceUDN);
			valueNode = deviceNode->first_node("friendlyName", 0, false);
			if (valueNode)
				deviceFriendlyName = valueNode->value();
			valueNode = deviceNode->first_node("modelDescription", 0, false);
			if (valueNode)
				deviceModelDescription = valueNode->value();
			valueNode = deviceNode->first_node("modelName", 0, false);
			if (valueNode)
				deviceModelName = valueNode->value();
			valueNode = deviceNode->first_node("manufacturer", 0, false);
			if (valueNode)
				deviceManufacturer = valueNode->value();
			valueNode = deviceNode->first_node("manufacturerURL", 0, false);
			if (valueNode)
				deviceManufacturerUrl = valueNode->value();
			valueNode = deviceNode->first_node("modelNumber", 0, false);
			if (valueNode)
				deviceModelNumber = valueNode->value();
			valueNode = deviceNode->first_node("serialNumber", 0, false);
			if (valueNode)
				deviceSerialNumber = valueNode->value();


			// devices will do some logging
			_upnpDevice->SetLoggerObject(this->GetLoggerObject());

			_upnpDevice->SetUDN(deviceUDN);
			_upnpDevice->SetDeviceType(deviceType);
			_upnpDevice->SetFriendlyName(deviceFriendlyName);
			_upnpDevice->SetModelDescription(deviceModelDescription);
			_upnpDevice->SetModelName(deviceModelName);
			_upnpDevice->SetModelNumber(deviceModelNumber);
			_upnpDevice->SetSerialNumber(deviceSerialNumber);
			_upnpDevice->SetManufacturer(deviceManufacturer);
			_upnpDevice->SetManufacturerUrl(deviceManufacturerUrl);

			// TODO: read icon list and store icons	(as bin data?, load icons in thread?)		

			// TODO: in futere we should read the 'raumfeld' information too
			// such like 'hardwareType, protocolVersion'
		}


		// DeviceManager::CreateMediaServerObjectFromDeviceXML
		MediaServer* DeviceManager::CreateMediaServerObjectFromDeviceXML(std::string _deviceXML)
		{
			xml_document<> doc;
			std::string deviceManufacturer;
			xml_node<> *deviceNode, *rootNode, *valueNode;				
			MediaServer *mediaServer;
			bool isRaumfeldMediaServer;

			// to parse the string we have to put it ino char* (because c_str() returns const char*)
			char* cstr = new char[_deviceXML.size() + 1];
			strcpy(cstr, _deviceXML.c_str());
			doc.parse<0>(cstr);

			// find the root node which has to be the 'device' node	
			rootNode = doc.first_node("root", 0, false);
			if (!rootNode)
			{
				this->Log(LogType::LOGERROR, "Requested device XML from device does not contain root block!", __FUNCTION__);
				return nullptr;
			}

			// find the root node which has to be the 'device' node	
			deviceNode = rootNode->first_node("device", 0, false);
			if (!deviceNode)
			{
				this->Log(LogType::LOGERROR, "Requested device XML from device does not contain device information!", __FUNCTION__);
				return nullptr;
			}

			valueNode = deviceNode->first_node("manufacturer", 0, false);
			if (valueNode)
				deviceManufacturer = valueNode->value();

			isRaumfeldMediaServer = deviceManufacturer.find(raumkernSettings.RAUMKERNEL_RAUMFELDMANUFACTURER) != std::string::npos;
			
			if (isRaumfeldMediaServer)
				mediaServer = new RaumfeldMediaServer();
			else
				mediaServer = new MediaServer();

			this->SetValuesUPNPDevice(mediaServer, deviceNode);
		
			return mediaServer;		
		}
	
		
		// DeviceManager::CreateMediaRendererObjectFromDeviceXML
		MediaRenderer* DeviceManager::CreateMediaRendererObjectFromDeviceXML(std::string _deviceXML)
		{
			xml_document<> doc;
			xml_node<> *deviceNode, *rootNode, *valueNode;
			std::string deviceModelDescription, deviceManufacturer;/*, deviceManufacturerUrl, deviceModelName, deviceType, deviceFriendlyName;*/
			//std::string	deviceModelNumber, deviceSerialNumber;
			bool isVirtualRenderer, isRaumfeldRenderer;
			MediaRenderer *mediaRenderer;

			// to parse the string we have to put it ino char* (because c_str() returns const char*)
			char* cstr = new char[_deviceXML.size() + 1];
			strcpy(cstr, _deviceXML.c_str());
			doc.parse<0>(cstr);
		
			// find the root node which has to be the 'device' node	
			rootNode = doc.first_node("root", 0, false);
			if (!rootNode)
			{
				this->Log(LogType::LOGERROR, "Requested device XML from device does not contain root block!", __FUNCTION__);
				return nullptr;
			}

			// find the root node which has to be the 'device' node	
			deviceNode = rootNode->first_node("device", 0, false);
			if (!deviceNode)
			{
				this->Log(LogType::LOGERROR, "Requested device XML from device does not contain device information!", __FUNCTION__);
				return nullptr;
			}

			valueNode = deviceNode->first_node("manufacturer", 0, false);
			if (valueNode)
				deviceManufacturer = valueNode->value();
			valueNode = deviceNode->first_node("modelDescription", 0, false);
			if (valueNode)
				deviceModelDescription = valueNode->value();					

			isVirtualRenderer	= deviceModelDescription.find(raumkernSettings.RAUNKERNEL_DESCRIPTIONVIRTUALMEDIAPLAYER) != std::string::npos;
			isRaumfeldRenderer = deviceManufacturer.find(raumkernSettings.RAUMKERNEL_RAUMFELDMANUFACTURER) != std::string::npos;
				 
			if (isRaumfeldRenderer)
			{
				if (isVirtualRenderer)
					mediaRenderer = new RaumfeldVirtualMediaRenderer();				
				else
					mediaRenderer = new RaumfeldMediaRenderer();
			}
			else
				mediaRenderer = new MediaRenderer();

			this->SetValuesUPNPDevice(mediaRenderer, deviceNode);
			
			// subscribe to media renderer state change! if media renderer comes to another state or data is updated, it will be called!						
			mediaRenderer->SubscribeSignalMediaRendererStateChanged(typeSignalMediaRendererStateChanged::slot_type(boost::bind(&DeviceManager::OnMediaRendererStateChanged, this, _1))); 

			return mediaRenderer;
		}

		
		// DeviceManager::GetCpDevice
		CpDeviceCpp* DeviceManager::GetCpDevice(std::string _udn)
		{				
			boost::unordered_map<std::string, OpenHome::Net::CpDeviceCpp*>::iterator it = upnpDeviceMap.find(Utils::FormatUDN(_udn));
			if (it != upnpDeviceMap.end())
				return it->second;
			return nullptr;
		}


		// DeviceManager::GetRenderer
		MediaRenderer* DeviceManager::GetMediaRenderer(std::string _udn)
		{	
			boost::unordered_map<std::string, MediaRenderer*>::iterator it = mediaRendererMap.find(Utils::FormatUDN(_udn));
			if (it != mediaRendererMap.end())
				return it->second;
			
			boost::unordered_map<std::string, MediaRenderer*>::iterator itVirtual = mediaRendererVirtualMap.find(Utils::FormatUDN(_udn));
			if (itVirtual != mediaRendererVirtualMap.end())
				return itVirtual->second;

			return nullptr;
		}


		// DeviceManager::GetMediaServer
		MediaServer* DeviceManager::GetMediaServer(std::string _udn)
		{
			if (_udn.empty())
			{			
				boost::unordered_map<std::string, MediaServer*>::iterator itFirst = mediaServerMap.begin();
				if (itFirst != mediaServerMap.end())
					return itFirst->second;
				return nullptr;
			}

			boost::unordered_map<std::string, MediaServer*>::iterator it = mediaServerMap.find(Utils::FormatUDN(_udn));
			if (it != mediaServerMap.end())
				return it->second;			
			return nullptr;
		}


		// OBSOLETE METHOD!
		// DeviceManager::RequestDeviceDetailXML
		std::string DeviceManager::RequestDeviceDetailXML(std::string _deviceUri)
		{
			std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> httpResponse;
			std::ostringstream ss;
			std::string responseBody;

			try
			{
				this->Log(LogType::LOGDEBUG, "Requesting Device Detail...", __FUNCTION__);
				// request zones from raumfeld host
				httpResponse = this->Request(_deviceUri, nullptr, nullptr, false, 20);
				ss << httpResponse->content.rdbuf();
				responseBody = ss.str();
				return responseBody;
			}
			catch (std::exception &e)
			{
				std::string exceptionText = e.what();
				this->Log(LogType::LOGERROR, exceptionText + "'" + _deviceUri + "'", __FUNCTION__);	
			}

			return "";
		}


		// OBSOLETE METHOD!
		// DeviceManager::RequestDevices	
		std::string DeviceManager::RequestDevices(std::string _longPollingId)
		{			
 			std::string longPollingUpdateId;
			std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> httpResponse;
			boost::unordered_map<std::string, std::string> headerVars;
			std::ostringstream ss;
			std::string responseBody;

			try
			{
				// request zones from raumfeld host
				headerVars.insert(std::make_pair("updateId", _longPollingId));
		
				httpResponse = this->Request("listDevices", &headerVars, nullptr);

				ss << httpResponse->content.rdbuf();
				responseBody = ss.str();

				// parse devices xml and signal change				
				this->ParseDevices(responseBody);

				// get 'updateId' from response header for 'long polling' option	
				longPollingUpdateId = this->GetLongPollingIdFromResponse(httpResponse);
			}
			catch (std::exception &e)
			{
				this->Log(LogType::LOGERROR, e.what(), __FUNCTION__);	
			}
			catch (boost::thread_interrupted const&)
			{
				// thread interrupted... 
				longPollingUpdateId = LONGPOLLINGREQUESTQUIT;
			}

			return longPollingUpdateId;
		}
		

		// DeviceManager::StartFindConfigDevice
		void DeviceManager::StartFindDevices()
		{
			OpenHome::Net::CpDeviceListCppUpnpAll* listAll;
			OpenHome::Net::FunctorCpDeviceCpp functorDeviceFound = OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &DeviceManager::OnDeviceFound);
			OpenHome::Net::FunctorCpDeviceCpp functorDeviceLost = OpenHome::Net::MakeFunctorCpDeviceCpp(*this, &DeviceManager::OnDeviceLost);
			// Search for all UPNP devices
			listAll = new OpenHome::Net::CpDeviceListCppUpnpAll(functorDeviceFound, functorDeviceLost);
		}


		// DeviceManager::CreateMediaRendererObjectFromDeviceXML
		UPNPDevice* DeviceManager::CreateUPNPDeviceObjectFromDeviceXML(std::string _deviceXML)
		{
			xml_document<> doc;
			xml_node<> *deviceNode, *rootNode, *valueNode;		
			MediaRenderer* mediaRenderer;
			MediaServer *mediaServer;
			std::string deviceType, deviceUDN;
			UPNPDevice *upnpDevice = nullptr;


			// to parse the string we have to put it ino char* (because c_str() returns const char*)
			char* cstr = new char[_deviceXML.size() + 1];
			strcpy(cstr, _deviceXML.c_str());
			doc.parse<0>(cstr);
				
			// find the root node which has to be the 'device' node	
			rootNode = doc.first_node("root", 0, false);
			if (!rootNode)
			{
				this->Log(LogType::LOGERROR, "Device XML from device does not contain root block!", __FUNCTION__);
				return nullptr;
			}

			// find the root node which has to be the 'device' node	
			deviceNode = rootNode->first_node("device", 0, false);
			if (!deviceNode)
			{
				this->Log(LogType::LOGERROR, "Device XML from device does not contain device information!", __FUNCTION__);
				return nullptr;
			}

			valueNode = deviceNode->first_node("udn", 0, false);
			if (!valueNode)
			{
				this->Log(LogType::LOGERROR, "Device XML from device does not contain UDN information!", __FUNCTION__);
				return nullptr;
			}
			deviceUDN = valueNode->value();

			valueNode = deviceNode->first_node("deviceType", 0, false);
			if (!valueNode)
			{
				this->Log(LogType::LOGERROR, "Device XML from device does not contain type information!", __FUNCTION__);
				return nullptr;
			}		
			deviceType = valueNode->value();

			if (deviceType.find(raumkernSettings.RAUMKERNEL_MEDIARENDERER_IDENTIFICATION) != std::string::npos)
			{
				this->Log(LogType::LOGDEBUG, "Create Object for Device (" + deviceUDN + ")", __FUNCTION__);
				mediaRenderer = this->CreateMediaRendererObjectFromDeviceXML(_deviceXML);
				if (mediaRenderer)
				{
					upnpDevice = mediaRenderer;
				}
				else
				{
					this->Log(LogType::LOGERROR, "Object for device '" + deviceType + "' with UDN: '" + deviceUDN + "' can not be created", __FUNCTION__);
				}			
			}
			// check if device is of type mediaServer
			// in fact there can only be one raumfeld media server we can use
			else if (deviceType.find(raumkernSettings.RAUMKERNEL_MEDIASERVER_IDENTIFICATION) != std::string::npos)
			{
				mediaServer = this->CreateMediaServerObjectFromDeviceXML(_deviceXML);
				if (mediaServer)
				{
					if (mediaServer->IsRaumfeldMediaServer())
					{						
						upnpDevice = mediaServer;
					}
					else
					{
						// we can not do anything with a media server which is no raumfeld media server, so....
						// maybee in future we can us it in any way... But for now....
						delete mediaServer;
					}
				}
				else
				{
					this->Log(LogType::LOGERROR, "Object for device '" + deviceType + "' with UDN: '" + deviceUDN + "' can not be created", __FUNCTION__);
				}
			}

			return upnpDevice;
		}


		// DeviceManager::DeviceFound
		// will be called if a UPNP Device is found in the network
		void DeviceManager::OnDeviceFound(OpenHome::Net::CpDeviceCpp& _device)
		{			
			bool deviceAlreadyInList = false;
			std::string friendlyName, location, deviceXML, deviceUDN;
			UPNPDevice *upnpDevice = nullptr;		
	
			this->Lock();

			try
			{
				deviceUDN = Utils::FormatUDN(_device.Udn());

				_device.GetAttribute("Upnp.DeviceXml", deviceXML);
				_device.GetAttribute("Upnp.Location", location);
				_device.GetAttribute("Upnp.FriendlyName", friendlyName);												

				this->Log(LogType::LOGDEBUG, "Found device: " + friendlyName, __FUNCTION__);
				
				// we add a refrence of the device to our internal list and to be sure we do not store it twice we do delete
				// all devices with the same UDN before. This should normaly not happen because "onDeviceLost" should be called before
				this->RemoveDeviceClassRef(deviceUDN);
				this->RemoveDeviceRef(deviceUDN);

				// create the proper UPNPDevice class derviate and add it to the clasDevice ref map
				// we do set a pointer link to the CPDevice the UPNPDevice is attached to
				upnpDevice = this->CreateUPNPDeviceObjectFromDeviceXML(deviceXML);
				if (upnpDevice)
				{
					upnpDevice->SetCpDevice(&_device);					
					this->AddDeviceClassRef(upnpDevice);
				}				
				
				this->AddDeviceRef(_device);							
				_device.AddRef();

			}
			catch (std::exception &ex)
			{	
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
			catch (const std::string& ex)
			{
				this->Log(LogType::LOGERROR, ex, __FUNCTION__);
			}
			catch (...)
			{	
				this->Log(LogType::LOGERROR, "Unresolved Error", __FUNCTION__);
			}

			this->UnLock();
					

			if (friendlyName == raumkernSettings.RAUMKERNEL_FRIENDLYNAME_CONFIGDEVICE)
			{
				this->Log(LogType::LOGINFO, "Configuration service found! (Location: " + location + ")", __FUNCTION__);
				this->OnConfigDeviceFound(_device);
			}
				
			// if we found a raumfeld media server, then signal out!
			if (upnpDevice && dynamic_cast<RaumfeldMediaServer*>(upnpDevice))
			{
				// if we found the raumfeld media server, the content manager has to subscribe to it, so that the content manager is aware of list changes, ect...
				managerList.contentManager->SubscribeToMediaServer();
				signalMediaServerFound();
			}

			signalDeviceListChanged();
		}


		// DeviceManager::DeviceLost
		// will be called if a UPNP Device disappears from the network
		void DeviceManager::OnDeviceLost(OpenHome::Net::CpDeviceCpp& _device)
		{					
			std::string friendlyName, location;
			std::string deviceUDN;
			bool mediaServerRemoved = false;

			this->Lock();

			try
			{
				deviceUDN = Utils::FormatUDN(_device.Udn());

				_device.GetAttribute("Upnp.Location", location);
				_device.GetAttribute("Upnp.FriendlyName", friendlyName);

				this->Log(LogType::LOGDEBUG, "Lost device: " + friendlyName, __FUNCTION__);	

				// remove the DeviceClass (eg. MediaRenderer, MediaServer,..) and the device reference (CPdevice) itself
				if (this->GetMediaServer(deviceUDN))
					mediaServerRemoved = true;

				this->RemoveDeviceClassRef(deviceUDN);
				this->RemoveDeviceRef(deviceUDN);

			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
			catch (const std::string& ex)
			{
				this->Log(LogType::LOGERROR, ex, __FUNCTION__);
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "Unresolved Error", __FUNCTION__);
			}

			this->UnLock();

			if (friendlyName == raumkernSettings.RAUMKERNEL_FRIENDLYNAME_CONFIGDEVICE)
			{
				this->Log(LogType::LOGWARNING, "Configuration service lost! (Location: " + location + ")", __FUNCTION__);
				this->OnConfigDeviceLost(_device);
			}
			
			if (mediaServerRemoved)
				signalMediaServerFound();

			signalDeviceListChanged();
			
		}


		// DeviceManager::RemoveDeviceClassRef
		void DeviceManager::RemoveDeviceClassRef(std::string _deviceUDN)
		{			
			//for (auto it = mediaRendererVirtualMap.find(_deviceUDN); it != mediaRendererVirtualMap.end(); it++)
			for (auto it = mediaRendererVirtualMap.begin(); it != mediaRendererVirtualMap.end(); it++)
			{	
				if (it->first == _deviceUDN)
				{
					delete it->second;
				}
			}

			//for (auto it = mediaRendererMap.find(_deviceUDN); it != mediaRendererMap.end(); it++)
			for (auto it = mediaRendererMap.begin(); it != mediaRendererMap.end(); it++)
			{
				if (it->first == _deviceUDN)
				{
					delete it->second;
				}
			}

			//for (auto it = mediaServerMap.find(_deviceUDN); it != mediaServerMap.end(); it++)
			for (auto it = mediaServerMap.begin(); it != mediaServerMap.end(); it++)
			{
				if (it->first == _deviceUDN)
				{
					delete it->second;
				}
			}

			mediaRendererVirtualMap.erase(_deviceUDN);
			mediaRendererMap.erase(_deviceUDN);
			mediaServerMap.erase(_deviceUDN);			
		}


		// DeviceManager::AddDeviceClassRef
		void DeviceManager::AddDeviceClassRef(UPNPDevice* _upnpDevice)
		{
			MediaServer* mediaServer = dynamic_cast<MediaServer*>(_upnpDevice);
			RaumfeldVirtualMediaRenderer* mediaRendererVirtual = dynamic_cast<RaumfeldVirtualMediaRenderer*>(_upnpDevice);
			
			if (mediaServer)
			{
				mediaServerMap.insert(std::make_pair(_upnpDevice->GetUDN(), (MediaServer*)mediaServer));
			}
			else if (mediaRendererVirtual)
			{
				mediaRendererVirtualMap.insert(std::make_pair(_upnpDevice->GetUDN(), (MediaRenderer*)mediaRendererVirtual));
			}
			else
			{
				mediaRendererMap.insert(std::make_pair(_upnpDevice->GetUDN(), (MediaRenderer*)_upnpDevice));
			}
		}


		// DeviceManager::RemoveDeviceRef
		void DeviceManager::RemoveDeviceRef(std::string _deviceUDN)
		{
			OpenHome::Net::CpDeviceCpp* device = nullptr;

			//for (auto it = upnpDeviceMap.find(_deviceUDN); it != upnpDeviceMap.end(); it++)
			for (auto it = upnpDeviceMap.begin(); it != upnpDeviceMap.end(); it++)
			{
				if (Utils::FormatUDN(it->first) == _deviceUDN)
				{
					device = it->second;
					device->RemoveRef();
				}
			}			
			upnpDeviceMap.erase(_deviceUDN);
		}


		// DeviceManager::AddDeviceRef
		void DeviceManager::AddDeviceRef(OpenHome::Net::CpDeviceCpp& _device)
		{
			upnpDeviceMap.insert(std::make_pair(Utils::FormatUDN(_device.Udn()), &_device));
		}


		// DeviceManager::ConfigDeviceFound
		// will be called if the RaumfeldConfig device appears on the network. 
		// we have to do some stuff for init to control the raumfeld system, then we are able to dignal that System is ready
		void DeviceManager::OnConfigDeviceFound(OpenHome::Net::CpDeviceCpp& _device)
		{
			signalConfigDeviceFound(_device);
		}


		// DeviceManager::ConfigDeviceLost
		// will be called if the RaumfeldConfig device disappears on the network. 
		// we have to signal that system is now "not ready"
		void DeviceManager::OnConfigDeviceLost(OpenHome::Net::CpDeviceCpp& _device)
		{
			signalConfigDeviceLost(_device);
		}
		

	
}
