#include "ZoneManager.h"
#include "DeviceManager.h"


namespace Raumkernel
{
	
		ZoneManager::ZoneManager() : HTTPRequestManager()
		{	
		}

		ZoneManager::~ZoneManager()
		{
		}

		// ZoneManager::IsListLockedForUpdate		
		bool ZoneManager::IsListLockedForUpdate()
		{
			if (mutexZoneListUpdate.try_lock())
			{
				mutexZoneListUpdate.unlock();
				return false;
			}
			else
				return true;				
		}

		// ZoneManager::WaitTillListIsReady		
		void ZoneManager::WaitTillListIsReady()
		{
			while (this->IsListLockedForUpdate())
			{
				this->Sleep(10);
			}
		}

		// ZoneManager::LockListForUpdate	
		void ZoneManager::LockListForUpdate(bool _lock)
		{
			if (_lock)
			{
				mutexZoneListUpdate.lock();
			}
			else
			{
				mutexZoneListUpdate.unlock();
			}

		}

		// ZoneManager::Lock	
		void ZoneManager::Lock()
		{
			LockListForUpdate(true);
		}

		// ZoneManager::UnLock	
		void ZoneManager::UnLock()
		{
			LockListForUpdate(false);
		}


		//ZoneManager::GetLastUpdateId
		boost::uint32_t ZoneManager::GetLastUpdateId()
		{
			return lastUpdateId;
		}


		// ZoneManager::StartRequestZonesWorkerThread
		// starts the thread for getting the zones list. Can be long polling if long polling id is present
		void ZoneManager::StartRequestZonesWorkerThread(std::string _longPollingUpdateId)
		{
			this->Log(LogType::LOGDEBUG, "Starting worker thread for zone requests...", __FUNCTION__);
			
			if (requestZonesThread.joinable())
			{
				requestZonesThread.interrupt();
				requestZonesThread.join();
			}
			requestZonesThread = boost::thread(&ZoneManager::RequestZonesWorkerThread, this, _longPollingUpdateId);	
		}

		// ZoneManager::RequestZonesWorkerThread
		// worker thread for polling and long polling the zone configuration of the system. 
		// Method will automatically request again id previouse request is done
		void ZoneManager::RequestZonesWorkerThread(std::string _longPollingUpdateId)
		{
			std::string longPollingUpdateId = _longPollingUpdateId;

			do
			{
				longPollingUpdateId = this->RequestZones(longPollingUpdateId);
			} while (longPollingUpdateId != LONGPOLLINGREQUESTQUIT);

			this->Log(LogType::LOGDEBUG, "RequestZonesWorkerThread finished", __FUNCTION__);
		}


		// ZoneManager::GetZoneRenderer	
		RaumfeldVirtualMediaRenderer* ZoneManager::GetZoneRenderer(std::string _zoneUDN)
		{
			if (this->GetDeviceManager())
			{
				return (RaumfeldVirtualMediaRenderer*)this->GetDeviceManager()->GetMediaRenderer(Utils::FormatUDN(_zoneUDN));
			}
			return nullptr;
		}


		// ZoneManager::ParseZoneConfiguration
		void ZoneManager::ParseZoneConfiguration(std::string _zonesXML)
		{
			xml_document<> doc;
			xml_node<> *zoneConfig, *zones, *unassignedRoomsNode;
			
			boost::mutex::scoped_lock scoped_lock(mutexZoneListUpdate);

			this->Log(LogType::LOGDEBUG, "Zone configuration changed! Parsing new data", __FUNCTION__);
						
			// remove lists because we do create them here again with the new values (may be more or less items)
			this->ClearZoneInformationMap();
			this->ClearRoomInformationMap();
			this->ClearRoomNameMap();			

			// to parse the string we have to put it ino char* (because c_str() returns const char*)
			char* cstr = new char[_zonesXML.size() + 1];
			strcpy(cstr, _zonesXML.c_str());
			doc.parse<0>(cstr);

			// find the root node which has to be the 'device' node	
			zoneConfig = doc.first_node("zoneConfig", 0, false);
			if (!zoneConfig)
			{
				this->Log(LogType::LOGERROR, "Requested zome XML  does not contain 'zoneConfig' block!", __FUNCTION__);
				return;
			}
			
			// do zones and rooms in zones
			zones = zoneConfig->first_node("zones", 0, false);
			if (zones)
			{				
				this->AddZoneInformationFromXmlNode(zones);
			}

			// do unassigned rooms
			unassignedRoomsNode = zoneConfig->first_node("unassignedRooms", 0, false);
			if (unassignedRoomsNode)
			{				
				this->AddRoomInformationFromXmlNode(unassignedRoomsNode);
			}	

			// each update of the zones we generate and store a random number
			// this is needed for JSON webserver for long polling action on zone information
			boost::uint32_t curUpdateId = Utils::GetRandomNumber();
			if (lastUpdateId == curUpdateId)
				lastUpdateId++;
			else
				lastUpdateId = curUpdateId;

			signalZoneConfigurationChanged();
		}


		// ZoneManager::AddRoomInformationFromXmlNode
		void ZoneManager::AddRoomInformationFromXmlNode(xml_node<>* _parnetNode, ZoneInformation *_zoneInformation)
		{
			xml_node<> *roomNode, *rendererNode;
			xml_attribute<> *attribute;
			std::string roomColor = "", roomUDN = "", roomName = "", rendererUDN = "";

			roomNode = _parnetNode->first_node("room", 0, false);
			while (roomNode)
			{
				RoomInformation		roomInfo;

				attribute = roomNode->first_attribute("udn", 0, false);
				if (attribute) roomUDN = attribute->value();

				attribute = roomNode->first_attribute("color", 0, false);
				if (attribute) roomColor = attribute->value();

				attribute = roomNode->first_attribute("name", 0, false);
				if (attribute) roomName = attribute->value();

				roomInfo.UDN = roomUDN;
				roomInfo.zoneUDN = "";
				roomInfo.name = roomName;
				roomInfo.color = roomColor;				

				rendererNode = roomNode->first_node("renderer", 0, false);
				while (rendererNode)
				{
					attribute = rendererNode->first_attribute("udn", 0, false);
					if (attribute) rendererUDN = attribute->value();

					roomInfo.rendererUDN.push_back(rendererUDN);

					rendererNode = rendererNode->next_sibling("renderer", 0, false);
				}

				if (_zoneInformation)
				{
					roomInfo.zoneUDN = _zoneInformation->UDN;
					_zoneInformation->roomsUDN.push_back(roomUDN);
				}

				roomInformationMap.insert(std::make_pair(roomInfo.UDN, roomInfo));
				roomNameUDNMap.insert(std::make_pair(roomInfo.name, roomInfo.UDN));

				roomNode = roomNode->next_sibling("room", 0, false);
			}
		}


		// ZoneManager::AddZoneInformationFromXmlNode
		void ZoneManager::AddZoneInformationFromXmlNode(xml_node<>* _parnetNode)
		{
			xml_node<> *zoneNode;
			xml_attribute<> *attribute;
			std::string zoneUDN, zoneName;
		
			zoneNode = _parnetNode->first_node("zone", 0, false);
			while (zoneNode)
			{
				ZoneInformation		zoneInfo;
				zoneName = "";

				attribute = zoneNode->first_attribute("udn", 0, false);
				if (attribute) zoneUDN = attribute->value();
				
				zoneInfo.UDN = zoneUDN;														

				this->AddRoomInformationFromXmlNode(zoneNode, &zoneInfo);

				// create name for zone
				for (auto &roomUdn : zoneInfo.roomsUDN)
				{
					RoomInformation roomInfo = roomInformationMap.at(roomUdn);
					if (!zoneName.empty())
						zoneName += ", ";
					zoneName += roomInfo.name;
				}
				
				zoneInfo.name = zoneName;

				zoneInformationMap.insert(std::make_pair(zoneInfo.UDN, zoneInfo));

				zoneNode = zoneNode->next_sibling("zone", 0, false);
			}
		}


		// ZoneManager::RequestZones	
		std::string ZoneManager::RequestZones(std::string _longPollingId)
		{	
			std::string longPollingUpdateId ="";
			std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> httpResponse;
			boost::unordered_map<std::string, std::string> headerVars;
			std::ostringstream ss;
			std::string responseBody;

			try
			{
				// request zones from raumfeld host
				headerVars.insert(std::make_pair("updateId", _longPollingId));
				httpResponse = this->Request("getZones", &headerVars, nullptr);
				if (httpResponse)
				{
					ss << httpResponse->content.rdbuf();
					responseBody = ss.str();

					// parse zone xml and signal change
					this->ParseZoneConfiguration(responseBody);

					// get 'updateId' from response header for 'long polling' option	
					longPollingUpdateId = this->GetLongPollingIdFromResponse(httpResponse);
				}
				else
				{
					longPollingUpdateId = LONGPOLLINGREQUESTQUIT;
				}
			}
			catch (std::exception &e)
			{
				this->Log(LogType::LOGERROR, e.what(), __FUNCTION__);
				this->Sleep(100);
			}
			catch (boost::thread_interrupted const&)
			{
				// thread interrupted... 
				longPollingUpdateId = LONGPOLLINGREQUESTQUIT;
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
			}

			return longPollingUpdateId;
		}


		// ZoneManager::SubscribeSignalZoneConfigurationChanged
		void ZoneManager::SubscribeSignalZoneConfigurationChanged(const typeSignalZoneConfigurationChanged::slot_type &_subscriber)
		{
			signalZoneConfigurationChanged.connect(_subscriber);
		}


		// ZoneManager::ClearZoneInformationMap
		void ZoneManager::ClearZoneInformationMap()
		{
			zoneInformationMap.clear();
		}

		// ZoneManager::ClearRoomNameMap
		void ZoneManager::ClearRoomNameMap()
		{
			roomNameUDNMap.clear();
		}

		// ZoneManager::ClearRoomInformationMap
		void ZoneManager::ClearRoomInformationMap()
		{
			roomInformationMap.clear();
		}
		

		// ZoneManager::ClearLists
		void ZoneManager::ClearLists()
		{
			boost::mutex::scoped_lock scoped_lock(mutexZoneListUpdate);

			this->ClearZoneInformationMap();
			this->ClearRoomInformationMap();
			this->ClearRoomNameMap();			

			signalZoneConfigurationChanged();
		}


		// ZoneManager::Reset
		void ZoneManager::Reset()
		{
			this->Log(LogType::LOGDEBUG, "Reseting Zone Manager...", __FUNCTION__);

			this->Log(LogType::LOGDEBUG, "Stopping zone polling thread", __FUNCTION__);

			if (requestZonesThread.joinable())
			{	
				// pragma for MSVC Compiler
				// seems to be that old versions of windows cant handle "cancel" on socket. (no problem since we do only release vista and above!)
				#pragma warning (disable: 4996)
				if (curRequestSocket)
				{
					this->Log(LogType::LOGDEBUG, "Cancel and close HTTP socket", __FUNCTION__);
					curRequestSocket->shutdown(boost::asio::ip::tcp::socket::shutdown_both);					
					curRequestSocket->cancel();
					curRequestSocket->close();
				}
				#pragma warning (default: 4996)				

				this->Log(LogType::LOGDEBUG, "Interrupting thread", __FUNCTION__);
				requestZonesThread.interrupt();
				requestZonesThread.join();

				this->Log(LogType::LOGDEBUG, "Zone polling thread stopped!", __FUNCTION__);
			}

			this->ClearLists();
			this->Log(LogType::LOGDEBUG, "Reset Done...", __FUNCTION__);
		}

		// ZoneManager::GetZoneInformation
		ZoneInformation ZoneManager::GetZoneInformation(std::string _zoneUDN)
		{	
			ZoneInformation empty;
			boost::unordered_map<std::string, ZoneInformation>::iterator it = zoneInformationMap.find(Utils::FormatUDN(_zoneUDN));
			if (it != zoneInformationMap.end())
				return it->second;
			return empty;
		}


		// ZoneManager::GetRoomInformation
		RoomInformation ZoneManager::GetRoomInformation(std::string _roomUDN)
		{
			RoomInformation empty;
			boost::unordered_map<std::string, RoomInformation>::iterator it = roomInformationMap.find(Utils::FormatUDN(_roomUDN));
			if (it != roomInformationMap.end())
				return it->second;
			return empty;
		}


		// ZoneManager::GetZoneFromRoom
		ZoneInformation ZoneManager::GetZoneInformationFromRoom(std::string _roomUDN)
		{
			ZoneInformation empty;
			boost::unordered_map<std::string, RoomInformation>::iterator it = roomInformationMap.find(Utils::FormatUDN(_roomUDN));
			if (it != roomInformationMap.end())
			{
				RoomInformation roomInfo = (RoomInformation)it->second;			
				boost::unordered_map<std::string, ZoneInformation>::iterator it = zoneInformationMap.find(Utils::FormatUDN(roomInfo.zoneUDN));
				if (it != zoneInformationMap.end())
					return it->second;
			}
			return empty;
		}


		// ZoneManager::GetRoomUDNFromRoomName
		std::string ZoneManager::GetRoomUDNFromRoomName(std::string _roomName)
		{
			//boost::mutex::scoped_lock scoped_lock(mutexZoneListUpdate);

			boost::unordered_map<std::string, std::string>::iterator it = roomNameUDNMap.find(_roomName);
			if (it != roomNameUDNMap.end())		
				return it->second;			
			return "";
		}


		// ZoneManager::GetRoomInformationFromRoomUDNOrName
		RoomInformation ZoneManager::GetRoomInformationFromRoomUDNOrName(std::string _roomNameOrUDN)
		{			
			RoomInformation roomInfo;
			std::string roomUDN;			

			try
			{

				// we first assume that the room was specified with the name
				roomUDN = this->GetRoomUDNFromRoomName(_roomNameOrUDN);
				if (roomUDN.empty())
					roomUDN = _roomNameOrUDN;
				roomInfo = this->GetRoomInformation(roomUDN);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}			

			return roomInfo;
		}


		// ZoneManager::GetZoneInformationMap
		boost::unordered_map<std::string, ZoneInformation>* ZoneManager::GetZoneInformationMap()
		{
			//boost::mutex::scoped_lock scoped_lock(mutexZoneListUpdate);

			return &zoneInformationMap;
		}


		// ZoneManager::GetRoomInformationMap
		boost::unordered_map<std::string, RoomInformation>* ZoneManager::GetRoomInformationMap()
		{
			//boost::mutex::scoped_lock scoped_lock(mutexZoneListUpdate);

			return &roomInformationMap;
		}


		// ZoneManager::ConnectRoomToZone
		void ZoneManager::ConnectRoomToZone(std::string _roomUDN, std::string _zoneUDN)
		{
			std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> httpResponse;
			boost::unordered_map<std::string, std::string> postVars;
			std::ostringstream ss;
			std::string responseBody;

			try
			{
				// request zones from raumfeld host
				if (!_roomUDN.empty())
					postVars.insert(std::make_pair("roomUDN", _roomUDN));
				if (!_zoneUDN.empty())
					postVars.insert(std::make_pair("zoneUDN", _zoneUDN));
				httpResponse = this->Request("connectRoomToZone", nullptr, &postVars);
				if (httpResponse)
				{
					ss << httpResponse->content.rdbuf();
					responseBody = ss.str();
				}
				
			}
			catch (std::exception &e)
			{
				this->Log(LogType::LOGERROR, e.what(), __FUNCTION__);
				this->Sleep(100);
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "UnKnown Error", __FUNCTION__);
			}
		}


		// ZoneManager::DropRoom
		void ZoneManager::DropRoom(std::string _roomUDN)
		{
			std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> httpResponse;
			boost::unordered_map<std::string, std::string> postVars;
			std::ostringstream ss;
			std::string responseBody;

			try
			{
				// request zones from raumfeld host
				postVars.insert(std::make_pair("roomUDN", _roomUDN));
				httpResponse = this->Request("dropRoomJob", nullptr, &postVars);			

				ss << httpResponse->content.rdbuf();
				responseBody = ss.str();

			}
			catch (std::exception &e)
			{
				this->Log(LogType::LOGERROR, e.what(), __FUNCTION__);
				this->Sleep(100);
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
			}
		}

	
}

