/*
 *
 * Known Issues
 * Special signs (non ascii) in room names will not be read correctly from the HTTP Request lib (will be done in the future from netlib) (for this rooms use the UDN instead of the name)
 * 
 *
 *
 *
 * TODO 
 *
 * Code kommentieren und schöner machen!!!
 * url.h und url.cpp.. benörigen wir das wirklich?
 *
 * Beenden des Raumservers throwed manachmal / segfaulted
 *
 * LINUXTEST
 *
 *
 * 28 Februar 2015 (Windows x86 + Linux x86 release + raumfeld base release)
 *
 * ERSTES RELEASE --> Linuxtext / Build u.Test auf LINUX X86 /  WINDOWS X86 / RAUMFELD HARDWARE (ARM)
 *
 *
 *
 * Webviews?
 *
 *
 * ZWEITES RELEASE / LINUX ARM (RaspberryPi) 
 * Linux ARM release? (Raspi?/cubietruck?)

 * OPTIONAL: remove compiler warnings on linux?!
 *
 * AutoUpdater (if not done yet)
 *
 * CurrentTrackChanged signal???
 * LoadUri sol metadaten laden
 *
 * OPTIONAL: request auf anderen class?
 * OPTIONAL: request action ausserhalb (überladen???)
 *
 * Choose Network adapter
 *
 *
 * ContentBrowser / Content Manager 
 *  extends ContentManager??? (eher nein, nur referenzen auf listenids???)
 *  mit Scope! (Napster, Meine Musik, usw... das suchergebnisse und browsing ok ist)
 *  History (pro scope (das wäre dan n der "zurück button")
 *  ...
 * tool Playlist save / restore (easy -> only ids / Hard -> Title + Artist)
 *
 * Linux ARM release? (Raspi?)
 * 
 * LINUXTEST
 *
 *
 * Renderer Images laden
 * MediaItems Images laden
 *
 *
 * Sleep Mode
 *
 *
 * Manjaro Linux
*/

#ifndef RAUMKERN_H
#define RAUMKERN_H


#include "os.h"

#include <string>
#include <fstream>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/network/protocol/http/client.hpp>
#include <boost/thread.hpp>

#include <rapidxml/rapidxml.hpp>

#include <OpenHome/Net/Cpp/OhNet.h>
#include <OpenHome/Net/Cpp/CpDevice.h>
#include <OpenHome/Net/Cpp/CpDeviceDv.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>

#include "ManagerList.h"

#include "Logger.h"
#include "RaumkernObject.h"
#include "Global.h"
#include "ZoneManager.h"
#include "DeviceManager.h"
#include "ContentManager.h"
#include "WebServer.h"
#include "MediaBrowser.h"

using namespace ApplicationLogging;
using namespace rapidxml;

using namespace boost::network;
using namespace boost::network::http;

using namespace Raumkernel;
using namespace Raumkernel::WebServer;

namespace Raumkernel
{
	
	typedef boost::signals2::signal<void()> typeSignalSystemReady;
	typedef boost::signals2::signal<void()> typeSignalSystemShutdown;		
	typedef boost::signals2::signal<void(LogData)> typeSignalRaumkernLog;

		
	class Raumkern
    {
        public:           
			EXPORT Raumkern();
			EXPORT virtual ~Raumkern();
			EXPORT bool Init();
			EXPORT void Log(LogType _logType, std::string _log, std::string _loaction = "", bool _forceLog = false);
			EXPORT void SetLogLevel(LogType _logType);			
			EXPORT void SetSettingsXMLFilePath(std::string _filePath);
			EXPORT void SetNetworkAdapterName(std::string _networkAdapterName);

			EXPORT Logger* GetLogger();
			EXPORT ZoneManager* GetZoneManager();
			EXPORT DeviceManager* GetDeviceManager();
			EXPORT RaumkernServer* GetRaumkernServer();
			EXPORT ContentManager* GetContentManager();

			EXPORT void SubscribeSignalSystemShutdown(const typeSignalSystemShutdown::slot_type &_subscriber);
			EXPORT void SubscribeSignalSystemReady(const typeSignalSystemReady::slot_type &_subscriber);
			EXPORT void SubscribeSignalZoneConfigurationChanged(const typeSignalZoneConfigurationChanged::slot_type &_subscriber);
			EXPORT void SubscribeSignaDeviceListChanged(const typeSignalDeviceListChanged::slot_type &_subscriber);
			EXPORT void SubscribeSignalLog(const typeSignalRaumkernLog::slot_type &_subscriber);
			EXPORT void SubscribeSignalMediaServerFound(const typeSignalMediaServerFound::slot_type &_subscriber);
			EXPORT void SubscribeSignalMediaServerLost(const typeSignalMediaServerLost::slot_type &_subscriber);			
			EXPORT void SubscribeSignalWebServerReady(const typeSignalWebServerReady::slot_type &_subscriber);
			EXPORT void SubscribeSignalWebServerShutdown(const typeSignalWebServerShutdown::slot_type &_subscriber);
			EXPORT void SubscribeSignalWebServerStartFailed(const typeSignalWebServerStartFailed::slot_type &_subscriber);
			
			EXPORT static std::string GetLogString(LogData _logData);	
			EXPORT std::vector<std::string> GetAvailableNetworkAdapterNames();
		
        protected:
			std::string	hostRequestUrl;
			std::string settingsXMLFilePath;
			std::string	networkAdapterName;

			bool InitUPNP();						
			void OnConfigDeviceFound(OpenHome::Net::CpDeviceCpp&);
			void OnConfigDeviceLost(OpenHome::Net::CpDeviceCpp&);
			void ResetRaumkern();
			bool LoadRaumkernSettings();
			void OnApplicationLog(ApplicationLogging::LogData _logData);									
			void OnAliveStateChanged(bool _isAlive);	

        private:
			typeSignalSystemReady					signalSystemReady;
			typeSignalSystemShutdown				signalSystemShutdown;			
			typeSignalRaumkernLog					signalLog;	
	
			Logger*				logger;			
			RaumkernServer*		raumkernServer;
			/*
			ZoneManager*		zoneManager;
			DeviceManager*		deviceManager;			
			AliveCheckManager*	aliveCheckManager;	
			*/

			std::vector<std::string>				networkAdapterNameList;
    };

}

#endif // RAUMKERN_H

