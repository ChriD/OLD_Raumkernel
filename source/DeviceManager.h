#ifndef DEVICEMANAGER_H_INCLUDED
#define DEVICEMANAGER_H_INCLUDED


#include "os.h"

#include <mutex> 

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/network/protocol/http/client.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>

#include <OpenHome/Net/Cpp/OhNet.h>
#include <OpenHome/Net/Cpp/CpDevice.h>
#include <OpenHome/Net/Cpp/CpDeviceDv.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include <rapidxml/rapidxml.hpp>

#include "Manager.h"
#include "Global.h"


using namespace ApplicationLogging;
using namespace rapidxml;

using namespace boost::network;
using namespace boost::network::http;

using namespace Raumkernel;


namespace Raumkernel
{	
		// base signals for raumkernel to get its state
		typedef boost::signals2::signal<void()> typeSignalDeviceListChanged;
		typedef boost::signals2::signal<void(OpenHome::Net::CpDeviceCpp&)> typeSignalConfigDeviceFound;
		typedef boost::signals2::signal<void(OpenHome::Net::CpDeviceCpp&)> typeSignalConfigDeviceLost;
		typedef boost::signals2::signal<void(const std::string)> typeSignalMediaRendererStateChanged; 
		typedef boost::signals2::signal<void()> typeSignalMediaServerFound;
		typedef boost::signals2::signal<void()> typeSignalMediaServerLost;

		// define some classes which will be include in each cpp file we need it
		class MediaRenderer;
		class RaumfeldMediaRenderer;
		class RaumfeldVirtualMediaRenderer;
		class MediaServer;
		class RaumfeldMediaServer;


		class DeviceManager : public HTTPRequestManager
		{
		public:
			DeviceManager();
			virtual ~DeviceManager();
			EXPORT void SubscribeSignalDeviceListChanged(const typeSignalDeviceListChanged::slot_type &_subscriber);
			EXPORT void SubscribeSignalConfigDeviceFound(const typeSignalConfigDeviceFound::slot_type &_subscriber);
			EXPORT void SubscribeSignalConfigDeviceLost(const typeSignalConfigDeviceLost::slot_type &_subscriber);	
			EXPORT void SubscribeSignalMediaRendererStateChanged(const typeSignalMediaRendererStateChanged::slot_type &_subscriber);
			EXPORT void SubscribeSignalMediaServerFound(const typeSignalMediaServerFound::slot_type &_subscriber);
			EXPORT void SubscribeSignalMediaServerLost(const typeSignalMediaServerLost::slot_type &_subscriber);

			EXPORT MediaRenderer* GetMediaRenderer(std::string _udn);
			EXPORT MediaServer* GetMediaServer(std::string _udn = "");			

			std::string RequestDevices(std::string _longPollingId = "");						
			void StartRequestDevicesWorkerThread(std::string _longPollingUpdateId = "");
			
			void StartFindDevices();			
			void Reset();			
					
			EXPORT void Lock();
			EXPORT void UnLock();
			EXPORT void LockRendererServerListForUpdate(bool _true);
			EXPORT bool IsRendererServerListLockedForUpdate();
			EXPORT void WaitTillRendererServerListIsReady();

			EXPORT boost::uint32_t GetLastUpdateIdRendererState();
			void SetLastUpdateIdRendererState(boost::uint32_t _lastUpdateIdRendererState);
			
		protected:
			void RequestDevicesWorkerThread(std::string _longPollingUpdateId);
			std::string RequestDeviceDetailXML(std::string _deviceUri);
			void ParseDevices(std::string _devicesXML);

			void SetValuesUPNPDevice(UPNPDevice* _upnpDevice, xml_node<> *_deviceNode);
			MediaRenderer* CreateMediaRendererObjectFromDeviceXML(std::string _deviceXML);
			MediaServer* CreateMediaServerObjectFromDeviceXML(std::string _deviceXML);		
			UPNPDevice* CreateUPNPDeviceObjectFromDeviceXML(std::string _deviceXML);

			void ClearLists();
			void ClearMediaRendererMap();
			void ClearUPNPDeviceMap();
			void ClearMediaServerMap();

			void RemoveDeviceClassRef(std::string _deviceUDN);
			void AddDeviceClassRef(UPNPDevice* _upnpDevice);
			void RemoveDeviceRef(std::string _deviceUDN);
			void AddDeviceRef(OpenHome::Net::CpDeviceCpp& _device);

			void OnMediaRendererStateChanged(std::string _rendererUDN);

			void OnDeviceFound(OpenHome::Net::CpDeviceCpp& _device);
			void OnDeviceLost(OpenHome::Net::CpDeviceCpp& _device);
			void OnConfigDeviceFound(OpenHome::Net::CpDeviceCpp& _device);
			void OnConfigDeviceLost(OpenHome::Net::CpDeviceCpp& _device);

			CpDeviceCpp* GetCpDevice(std::string _udn);
						

		private:
			typeSignalDeviceListChanged				signalDeviceListChanged;
			typeSignalConfigDeviceFound				signalConfigDeviceFound;
			typeSignalConfigDeviceLost				signalConfigDeviceLost;
			typeSignalMediaRendererStateChanged		signalMediaRendererStateChanged;
			typeSignalMediaServerFound				signalMediaServerFound;
			typeSignalMediaServerLost				signalMediaServerLost;

			boost::thread	requestDevicesThread;
			
			
			boost::mutex	mutexUPNPDeviceList;
			boost::mutex	mutexRendererServerListUpdate;
			boost::mutex	mutexUpdateIdRendererState;

			// this list holds references to all the UPNP devices found by the UPNP control point, no matter if they ar usable by raumfeld system or not
			boost::unordered_map<std::string, OpenHome::Net::CpDeviceCpp*>upnpDeviceMap;

			// this list holds information about the virtual media renderers. This are equivalent to 'zones'. It holds no CpDevice itself, only through link due 'uid'			
			boost::unordered_map<std::string, MediaRenderer*> mediaRendererVirtualMap;

			// this list holds information about all the media renderers (except virtual). It holds no CpDevice itself, only through link due 'uid'
			boost::unordered_map<std::string, MediaRenderer*> mediaRendererMap;

			// this list holds information about all raumfeld media servers. (in fact only one) It holds no CpDevice itself, only through link due 'uid'
			boost::unordered_map<std::string, MediaServer*>	mediaServerMap;

			// id of last update of any virtual renderer
			boost::uint32_t lastUpdateIdRendererState;

		};




}

#endif // DEVICEMANAGER_H_INCLUDED
