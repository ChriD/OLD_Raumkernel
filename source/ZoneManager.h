#ifndef ZONEMANAGER_H_INCLUDED
#define ZONEMANAGER_H_INCLUDED


#include "os.h"

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>

#include <rapidxml/rapidxml.hpp>

#include "Manager.h"
#include "Renderer.h"

using namespace ApplicationLogging;
using namespace rapidxml;


namespace Raumkernel
{

		typedef boost::signals2::signal<void()> typeSignalZoneConfigurationChanged;					

		class ZoneManager : public HTTPRequestManager
		{
		public:
			ZoneManager();
			virtual ~ZoneManager();
			EXPORT void SubscribeSignalZoneConfigurationChanged(const typeSignalZoneConfigurationChanged::slot_type &_subscriber);
			std::string RequestZones(std::string _longPollingId = "");
			void StartRequestZonesWorkerThread(std::string _longPollingUpdateId = "");		
			void Reset();

			// Puts the room with the given roomUDN in the zone with the zoneUDN
			// zoneUDN: The udn of the zone to connect the room to. If zone udn is empty, a new zone is created
			// roomUDN: The udn of the room that has to be put into that zone.If empty, all available rooms(rooms that have active renderers) are put into the zone.
			EXPORT void ConnectRoomToZone(std::string _roomUDN, std::string _zoneUDN);
			// Drops the room with the given roomUDN from the zone it is in. 
			EXPORT void DropRoom(std::string _roomUDN);

			EXPORT ZoneInformation GetZoneInformation(std::string _zoneUDN);			
			EXPORT ZoneInformation GetZoneInformationFromRoom(std::string _roomUDN);
			EXPORT RoomInformation GetRoomInformation(std::string _roomUDN);
			EXPORT RaumfeldVirtualMediaRenderer* GetZoneRenderer(std::string _zoneUDN);
			EXPORT std::string GetRoomUDNFromRoomName(std::string _roomName);
			EXPORT RoomInformation GetRoomInformationFromRoomUDNOrName(std::string _roomNameOrUDN);

			EXPORT boost::unordered_map<std::string, ZoneInformation>* GetZoneInformationMap();
			EXPORT boost::unordered_map<std::string, RoomInformation>* GetRoomInformationMap();

			EXPORT void Lock();
			EXPORT void UnLock();
			EXPORT void LockListForUpdate(bool _true);
			EXPORT bool IsListLockedForUpdate();
			EXPORT void WaitTillListIsReady();
			
			boost::uint32_t GetLastUpdateId();

		protected:			
			void RequestZonesWorkerThread(std::string _longPollingUpdateId);			
			void ParseZoneConfiguration(std::string _zonesXML);

			// clear methods for clearing lists
			void ClearLists();
			void ClearZoneInformationMap();
			void ClearRoomInformationMap();
			void ClearRoomNameMap();			

		private:
			typeSignalZoneConfigurationChanged		signalZoneConfigurationChanged;

			boost::mutex	mutexZoneListUpdate;

			// 
			boost::unordered_map<std::string, ZoneInformation>	zoneInformationMap;
			// this map holds a 'room name' to 'UDN' link
			boost::unordered_map<std::string, std::string>	roomNameUDNMap;			
			// room information map
			boost::unordered_map<std::string, RoomInformation>	roomInformationMap;

			boost::thread	requestZonesThread;

			// zone managers has to be aware of device manager
			//DeviceManager	*deviceManager;

			void AddRoomInformationFromXmlNode(xml_node<>* _parnetNode, ZoneInformation *_zoneInformation = nullptr);
			void AddZoneInformationFromXmlNode(xml_node<>* _parnetNode);

			boost::uint32_t lastUpdateId;
		};


	

}

#endif // ZONEMANAGER_H_INCLUDED
