#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "os.h"
#include "Global.h"

#include <string>
#include <iostream>
#include <vector>
#include <boost/network.hpp>
#include <rapidxml/rapidxml.hpp>
#include "RaumkernObject.h"
#include "webserver/server_http.hpp"
#include "WebServerResponseHandler.h"


using namespace SimpleWeb;

namespace Raumkernel
{
	namespace WebServer
	{

		enum class RequestActionScope { RAS_ZONE = 0, RAS_ROOM = 1};

		typedef boost::signals2::signal<void()> typeSignalWebServerReady;
		typedef boost::signals2::signal<void()> typeSignalWebServerShutdown;
		typedef boost::signals2::signal<void(std::string)> typeSignalWebServerStartFailed;


		// all actions wich are available are listed here in const objects
		const std::string ACTION_CREATEZONE = "createzone";
		const std::string ACTION_DROPFROMZONE = "dropfromzone";
		const std::string ACTION_ADDTOZONE = "addtozone";		
		const std::string ACTION_VOLUME = "volume";
		const std::string ACTION_MUTE = "mute";
		const std::string ACTION_UNMUTE = "unmute";
		const std::string ACTION_FADETOVOLUME = "fadetovolume";
		const std::string ACTION_VOLUMECHANGE = "volumechange";
		const std::string ACTION_PAUSE = "pause";
		const std::string ACTION_PLAYNEXT = "playnext";
		const std::string ACTION_PLAYPREV = "playprev";
		const std::string ACTION_PLAY = "play";
		const std::string ACTION_TOGGLEMUTE = "togglemute";
		const std::string ACTION_SLEEPTIMER = "sleeptimer";
		const std::string ACTION_WAKEUPTIMER = "wakeuptimer";
		const std::string ACTION_LOADCONTAINER = "loadcontainer";
		const std::string ACTION_LOADURI = "loaduri";
		const std::string ACTION_LOADSINGLE = "loadsingle";
		const std::string ACTION_TOGGLEPAUSE = "togglepause";
		const std::string ACTION_LOADPLAYLIST = "loadplaylist";
		const std::string ACTION_PLAYMODE = "playmode";

		// 250ms standard wait time for requests should be fine (we should stay on the save side)
		const boost::uint16_t ACTION_STANDARDWAITTIME = 250;


		struct RequestAction
		{
			std::string path;
			std::string roomUDN;
			std::string roomName;	
			std::string action;
			RequestActionScope scope;
			boost::unordered_map<std::string, std::string> queryValues;	
			boost::uint32_t waitAfterOperation;
			std::string zoneUDN;
		};

		struct RequestPreset
		{
			std::string path;
			std::string presetId;	
			std::string action;
			boost::unordered_map<std::string, std::string> queryValues;		
		};


		class RaumkernServer : public RaumkernObject
		{
			public:
				EXPORT RaumkernServer();
				EXPORT virtual ~RaumkernServer();
				
				EXPORT bool StartServer();
				EXPORT void StopServer();	
				EXPORT RequestAction GetRequestActionFromPath(std::string _path);
				EXPORT RequestPreset GetRequestPresetFromPath(std::string _path);

				EXPORT void SubscribeSignalWebServerReady(const typeSignalWebServerReady::slot_type &_subscriber);				
				EXPORT void SubscribeSignalWebServerShutdown(const typeSignalWebServerShutdown::slot_type &_subscriber);
				EXPORT void SubscribeSignalWebServerStartFailed(const typeSignalWebServerStartFailed::slot_type &_subscriber);
				
				Server<HTTP>* GetServerObject();

			protected:
				Server<HTTP>* server;			

				JSONResponseHandler jsonResponseHandler;

				void HandleRoomRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request);
				void HandleZoneRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request);
				void HandlePresetRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request);
				void HandleBadRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request);
				void HandleWebClientRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request);
				void HandleJSONRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request);

				void DoRequestAction(RequestAction _requestAction);
				void DoRequestPreset(RequestPreset _requestPreset);
				RequestActionScope GetScopeForAction(std::string _action, RequestActionScope _preferedScope);				
				bool VirtualRendererNeededForAction(std::string _action, RequestActionScope _preferedScope);
				std::string GetQueryValue(RequestAction _requestAction, std::string _key);

				void StartServerThread();

				void StartSleepTimerThread(std::string _zoneUDN, boost::uint32_t _sleepTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS);
				static void SleepTimerThread(std::string _zoneUDN, boost::uint32_t _sleepTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS, RaumkernServer &_class);
				void StartWakeUpTimerThread(std::string _zoneUDN, boost::uint32_t _wakeUpTimeDurationMS, boost::uint32_t _fadeInTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS, boost::uint8_t _maxVolume);
				static void WakeUpTimerThread(std::string _zoneUDN, boost::uint32_t _wakeUpTimeDurationMS, boost::uint32_t _fadeInTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS, boost::uint8_t _maxVolume, RaumkernServer &_class);
				void StartFadeToVolumeThread(std::string _zoneUDN, boost::uint32_t _fadeTimeMS, boost::uint8_t _volume);
				static void FadeToVolumeThread(std::string _zoneUDN, boost::uint32_t _fadeTimeMS, boost::uint8_t _volume, RaumkernServer &_class);

				void InterruptThreadsForZone(std::string _zoneUDN);

				void VolumeInDeCrease(std::string _zoneUDN, boost::uint32_t _timeToDoMS, boost::uint8_t _toVolume);

				bool LoadRequestPresetsFromXML(std::string _xml);
				bool LoadRequestPresets(bool _force = false);
				void InitStandardRequestwaitTimeMap();

				boost::uint32_t GetStandardWaitTimeForRequestAction(RequestAction _action);
				boost::int16_t GetVolumeValueFromRequestAction(RequestAction _requestAction);

				void ReplaceRequestPlaceholders(RequestAction &_requestAction, RequestPreset _requestPreset, bool _removeNotFound = true);
				void RemoveRequestPlaceholders(RequestAction &_requestAction);

			private:	
				boost::thread	startServerThread;

				// unordered map wich holds the thread elements each zone
				boost::unordered_map<std::string, boost::thread*>				zoneThreadMap;

				// map that holds all presets. will be loaded when 'loadRequestPresetsFromXML' will be called
				boost::unordered_map<std::string, std::vector<RequestAction>> requestPresetsMap;

				// map that holds the standard wait times for request actions (if no wait time was defined)
				boost::unordered_map<std::string, boost::int32_t> requestWaitMap;				

				typeSignalWebServerReady				signalWebserverReady;
				typeSignalWebServerShutdown				signalWebserverShutdown;
				typeSignalWebServerStartFailed			signalWebserverStartFailed;

				bool									forceLoadPresets;

				boost::mutex presetRunningLock;
				boost::mutex requestRunningLock;

		};
	}
}


#endif	// WEBSERVER_H


