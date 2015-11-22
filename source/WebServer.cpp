#include "WebServer.h"
#include "ZoneManager.h"
#include "DeviceManager.h"

namespace Raumkernel
{
	namespace WebServer
	{

		RaumkernServer::RaumkernServer() : RaumkernObject()
		{
			server = nullptr;	
			forceLoadPresets = false;
			this->InitStandardRequestwaitTimeMap();
		}

		RaumkernServer::~RaumkernServer()
		{		
			// stop server and waiting for server threads to be finished / aborted
			this->StopServer();
			// interrupt all threads which are running (stop them where they are)
			this->InterruptThreadsForZone("");
			// now we can safely delete our http server object
			if (server) delete server;

			// lets be sure (for linux)
			this->Sleep(1000);
		}

		Server<HTTP>* RaumkernServer::GetServerObject()
		{
			return server;
		}


		// request path looks like following examples	
		// eg.: /raumserver/room/Schlafzimmer/createZone			
		// eg.: /raumserver/room/Schlafzimmer/loadPlaylist?list=Test
		// eg.: /raumserver/room/Schlafzimmer/play?track=1
		// eg.: /raumserver/zone/Schlafzimmer/volumeUp
		// eg.: /raumserver/zone/Schlafzimmer/volumeUp?value=2
		RequestAction RaumkernServer::GetRequestActionFromPath(std::string _path)
		{
			RequestAction	requestAction;
			std::string		urlQuery, roomNameOrUDN, actionAndQuery;
			std::vector<std::string> urlVector, actionAndQueryVector;
			
			// vector positions of "raumserver" and scope ("zone" or "room") are always present! (we insured that with the regex values on server start)
			urlVector = Utils::ExplodeString(_path, "/");
			if (urlVector.size() < 4)
			{
				this->Log(LogType::LOGERROR, "PATH For Request not correct!", __FUNCTION__);
				return requestAction;
			}
		
			roomNameOrUDN  = Utils::Unescape(urlVector[3]);	
			actionAndQuery = Utils::Unescape(urlVector[4]);

			actionAndQueryVector = Utils::ExplodeString(actionAndQuery, "?");
			if (actionAndQueryVector.size() > 1)
				urlQuery = actionAndQueryVector[1];

			RoomInformation roomInfo = this->GetZoneManager()->GetRoomInformationFromRoomUDNOrName(roomNameOrUDN);					
			requestAction.path = _path;
			requestAction.roomName = roomInfo.name;
			requestAction.roomUDN = roomInfo.UDN;
			// do not store zone information because they can change after a set of requests!
			requestAction.action = actionAndQueryVector[0];
			boost::algorithm::to_lower(requestAction.action);
			requestAction.scope = Utils::Unescape(urlVector[2]) == "zone" ? RequestActionScope::RAS_ZONE : RequestActionScope::RAS_ROOM;
			requestAction.queryValues = Utils::ParseQueryString(urlQuery);

			// be sure we do not wait
			requestAction.waitAfterOperation = 0;

			// it may be that the UDN which is delivered by the reuwst is a zone UDN, so we search for it and if found we set the zone udn
			if (roomInfo.UDN.empty())
			{
				ZoneInformation zoneInfo = this->GetZoneManager()->GetZoneInformation(roomNameOrUDN);
				requestAction.zoneUDN = zoneInfo.UDN;
			}

			return requestAction;
		}

		RequestPreset RaumkernServer::GetRequestPresetFromPath(std::string _path)
		{
			RequestPreset	requestPreset;
			std::string		urlQuery, presetId, actionAndQuery;
			std::vector<std::string> urlVector, actionAndQueryVector;

			// vector positions of "raumserver" and preset marker ("preset") are always present! (we insured that with the regex values on server start)
			urlVector = Utils::ExplodeString(_path, "/");
			if (urlVector.size() < 4)
			{
				this->Log(LogType::LOGERROR, "PATH For Request not correct!", __FUNCTION__);
				return requestPreset;
			}

			presetId = Utils::Unescape(urlVector[3]);
			actionAndQuery = Utils::Unescape(urlVector[4]);

			actionAndQueryVector = Utils::ExplodeString(actionAndQuery, "?");
			if (actionAndQueryVector.size() > 1)
				urlQuery = actionAndQueryVector[1];	

			requestPreset.path = _path;	
			requestPreset.presetId = presetId;
			requestPreset.action = actionAndQueryVector[0];
			boost::algorithm::to_lower(requestPreset.presetId);
			boost::algorithm::to_lower(requestPreset.action);			
			requestPreset.queryValues = Utils::ParseQueryString(urlQuery);

			return requestPreset;
		}


		std::string RaumkernServer::GetQueryValue(RequestAction _requestAction, std::string _key)
		{
			boost::unordered_map<std::string, std::string>::iterator it = _requestAction.queryValues.find(_key);
			if (it != _requestAction.queryValues.end())
			{
				return it->second;
			}
			return "";
		}


		void RaumkernServer::SubscribeSignalWebServerReady(const typeSignalWebServerReady::slot_type &_subscriber)
		{
			signalWebserverReady.connect(_subscriber);
		}
	
		void RaumkernServer::SubscribeSignalWebServerShutdown(const typeSignalWebServerShutdown::slot_type &_subscriber)
		{
			signalWebserverShutdown.connect(_subscriber);
		}

		void RaumkernServer::SubscribeSignalWebServerStartFailed(const typeSignalWebServerStartFailed::slot_type &_subscriber)
		{
			signalWebserverStartFailed.connect(_subscriber);
		}
	
		// Available commands
		// /raumserver/room/Schlafzimmer/createZone?force=true
		// /raumserver/room/Schlafzimmer/dropFromZone
		// /raumserver/room/Schlafzimmer/addToZone?room=[roomUDN or roomName]
		// /raumserver/zone/Schlafzimmer/playNext
		// /raumserver/zone/Schlafzimmer/playPrev
		// /raumserver/zone/Schlafzimmer/play
		// /raumserver/zone/Schlafzimmer/play?track=[trackNumber]
		// /raumserver/zone/Schlafzimmer/pause
		// /raumserver/zone/Schlafzimmer/loadPlayList?name=[playlistName]
		// /raumserver/zone/Schlafzimmer/volume?value=[volumeValue (0..100)]
		// /raumserver/zone/Schlafzimmer/volumeChange?value=[volumeDiff (eg. -2, or 2)]
		// /raumserver/zone/Schlafzimmer/mute
		// /raumserver/zone/Schlafzimmer/unmute
		// /raumserver/zone/Schlafzimmer/togglemute
		// /raumserver/zone/Schlafzimmer/sleepTimer?time=1800&fadeOutTime=120
		// /raumserver/zone/Schlafzimmer/wakeUpTimer?time=1800&fadeInTime=300&maxVolume=75&fadeOutTime=10
		// /raumserver/zone/Schlafzimmer/fadeToVolume?value=65&fadeTime=5
		// /raumserver/zone/Schlafzimmer/playMode?value=1
		// /raumserver/zone/Schlafzimmer/loadContainer?cid=0%2FPlaylists%2FMyPlaylists%2FLounge%201
		// /raumserver/zone/Schlafzimmer/loadUri?uri=http%3A%2F%2F79.141.174.206%3A15000
		// /raumserver/zone/Schlafzimmer/togglepause
		void RaumkernServer::DoRequestAction(RequestAction _requestAction)
		{
			boost::uint16_t waitTime = 0;
			boost::uint16_t virtualRendererWaitCount = 0;
			ZoneInformation zoneInfo;

			if (_requestAction.path.empty())
				throw std::runtime_error("'path' for request not correct!");				
			if (_requestAction.roomUDN.empty() && _requestAction.zoneUDN.empty())
				throw std::runtime_error("room-name or room-udn or zone-udn could not be resolved");
			if (_requestAction.action.empty())
				throw std::runtime_error("no action defined!");

			_requestAction.scope = this->GetScopeForAction(_requestAction.action, _requestAction.scope);

			this->Log(LogType::LOGINFO, "Request received (" + _requestAction.path + ")", __FUNCTION__);

			// for doing the request we have to lock the device and zone manager so no updates are going to be done on their internal lists from other threads
			// this would be bad because those rebuild the internal device list maps and do delete / create the renderes and media server classes
			this->GetDeviceManager()->Lock();
			this->GetZoneManager()->Lock();


			try
			{

				// get room and zone information we need for this request
				RoomInformation roomInfo = this->GetZoneManager()->GetRoomInformation(_requestAction.roomUDN);
				if (!_requestAction.zoneUDN.empty())
					zoneInfo = this->GetZoneManager()->GetZoneInformation(_requestAction.zoneUDN);
				else
					zoneInfo = this->GetZoneManager()->GetZoneInformation(roomInfo.zoneUDN);

				RaumfeldVirtualMediaRenderer* zoneRenderer = (RaumfeldVirtualMediaRenderer*)this->GetDeviceManager()->GetMediaRenderer(zoneInfo.UDN);

				// if we need a virtual renderer for action we maybe have to wait until it appears (if we created a zone it may take a few seconds while we get notificated about it)
				// so we dó a while loop and meanwhile do release the lock of the managers so they can work and are not locked for the other threads (listDevices / requestZones)
				if (this->VirtualRendererNeededForAction(_requestAction.action, _requestAction.scope))
				{
					while (!zoneRenderer && virtualRendererWaitCount <= 20)
					{
						zoneRenderer = (RaumfeldVirtualMediaRenderer*)this->GetDeviceManager()->GetMediaRenderer(zoneInfo.UDN);
						if (zoneRenderer)
							break;

						this->GetDeviceManager()->UnLock();
						this->GetZoneManager()->UnLock();

						this->Sleep(500);

						this->GetDeviceManager()->Lock();
						this->GetZoneManager()->Lock();

						virtualRendererWaitCount++;
					}

					// if we there is an action which need a zone, then throw if we do not have a virtual renderer
					if (!zoneRenderer)
					{
						throw std::runtime_error("zone renderer not present! Action can not be performed!");
					}
				}


				if (_requestAction.action == ACTION_CREATEZONE)
				{
					std::string force = this->GetQueryValue(_requestAction, "force");
					if (force == "true" || roomInfo.zoneUDN.empty())
					{
						this->GetZoneManager()->ConnectRoomToZone(roomInfo.UDN, "");
					}
				}
				else if (_requestAction.action == ACTION_DROPFROMZONE)
				{
					this->GetZoneManager()->DropRoom(roomInfo.UDN);
				}
				else if (_requestAction.action == ACTION_ADDTOZONE)
				{
					std::string roomNameOrUDN = this->GetQueryValue(_requestAction, "room");
					std::string zoneUDN = this->GetQueryValue(_requestAction, "zone");
					if (zoneUDN.empty())
						zoneUDN = this->GetZoneManager()->GetRoomInformationFromRoomUDNOrName(roomNameOrUDN).zoneUDN;

					this->GetZoneManager()->ConnectRoomToZone(roomInfo.UDN, zoneUDN);
				}
				else if (_requestAction.action == ACTION_PLAYNEXT)
				{
					zoneRenderer->Next(true);
				}
				else if (_requestAction.action == ACTION_PLAYPREV)
				{
					zoneRenderer->Previous(true);
				}
				else if (_requestAction.action == ACTION_PLAY)
				{
					std::string trackNumber = this->GetQueryValue(_requestAction, "track");
					if (!trackNumber.empty())
						zoneRenderer->PlayTrack(Raumkernel::Utils::TrackNumberToTrackIndex(this->toInt(trackNumber)), true);
					else
						zoneRenderer->Play(true);
				}
				else if (_requestAction.action == ACTION_LOADPLAYLIST)
				{
					std::string playlistName = this->GetQueryValue(_requestAction, "name");
					if (playlistName.empty())
						throw std::runtime_error("no playlist name defined!");

					// playlist name is unescaped, we have to escape it for playlist playing
					playlistName = Utils::EncodeValue(playlistName);

					zoneRenderer->LoadContainer("0/Playlists/MyPlaylists/" + playlistName);
				}
				// set volume can be done either on room or zone, so we have to handle the scope here correctly
				else if (_requestAction.action == ACTION_VOLUME)
				{
					boost::uint8_t volume = (boost::uint8_t)this->GetVolumeValueFromRequestAction(_requestAction);
					if (_requestAction.scope == RequestActionScope::RAS_ZONE)
						zoneRenderer->SetVolume(volume);
					else
						zoneRenderer->SetRoomVolume(_requestAction.roomUDN, volume);
				}
				else if (_requestAction.action == ACTION_VOLUMECHANGE)
				{
					boost::int16_t volumeDiff = this->GetVolumeValueFromRequestAction(_requestAction);
					boost::int16_t volume;

					if (_requestAction.scope == RequestActionScope::RAS_ZONE)
					{
						volume = (zoneRenderer->GetMediaRendererState().volume + volumeDiff);
						if (volume > 100) volume = 100;
						if (volume < 0) volume = 0;
						zoneRenderer->SetVolume((boost::uint8_t)volume);
					}
					else
					{
						volume = (zoneRenderer->GetMediaRendererRoomState(_requestAction.roomUDN).volume + volumeDiff);
						if (volume > 100) volume = 100;
						if (volume < 0) volume = 0;
						zoneRenderer->SetRoomVolume(_requestAction.roomUDN, (boost::uint8_t)volume);
					}
				}
				else if (_requestAction.action == ACTION_MUTE)
				{
					if (_requestAction.scope == RequestActionScope::RAS_ZONE)
						zoneRenderer->SetMute(true);
					else
						zoneRenderer->SetRoomMute(_requestAction.roomUDN, true);
				}
				else if (_requestAction.action == ACTION_UNMUTE)
				{
					if (_requestAction.scope == RequestActionScope::RAS_ZONE)
						zoneRenderer->SetMute(false);
					else
						zoneRenderer->SetRoomMute(_requestAction.roomUDN, false);
				}
				else if (_requestAction.action == ACTION_TOGGLEMUTE)
				{
					if (_requestAction.scope == RequestActionScope::RAS_ZONE)
					{
						if (zoneRenderer->GetMediaRendererState().muteState == MediaRendererMuteState::MRPMUTE_ALL)
							zoneRenderer->SetMute(false);
						else if (zoneRenderer->GetMediaRendererState().muteState == MediaRendererMuteState::MRPMUTE_NONE)
							zoneRenderer->SetMute(true);
						// if partial muted, toggle does 'mute' 
						else if (zoneRenderer->GetMediaRendererState().muteState == MediaRendererMuteState::MRPMUTE_PARTIAL)
							zoneRenderer->SetMute(true);
					}
					else
					{
						zoneRenderer->SetRoomMute(_requestAction.roomUDN, !zoneRenderer->GetMediaRendererRoomState(_requestAction.roomUDN).mute);
					}
				}
				// example: sleepTimer?time=1800&fadeOutTime=120 (sleeptimer will be set to 30 minutues, then it fades out the current volume to 0 and then stops playing)
				else if (_requestAction.action == ACTION_SLEEPTIMER)
				{
					std::string timeStr = this->GetQueryValue(_requestAction, "time");
					std::string fadeOutTimeStr = this->GetQueryValue(_requestAction, "fadeouttime");

					if (timeStr.empty() || this->toInt(timeStr) < 0)
						throw std::runtime_error("no correct sleep time duration defined!");
					if (!fadeOutTimeStr.empty() && this->toInt(fadeOutTimeStr) < 0)
						throw std::runtime_error("no correct fadeOut time duration defined!");

					this->StartSleepTimerThread(zoneRenderer->GetUDN(), this->toInt(timeStr) * 1000, this->toInt(fadeOutTimeStr) * 1000);
				}
				// example: wakeUpTimer?time=1800&fadeInTime=300&maxvolume=65 (wakeup timer will fade in until volume 65 is reached. then it plays an half an hour before it stops again)
				else if (_requestAction.action == ACTION_WAKEUPTIMER)
				{
					std::string timeStr = this->GetQueryValue(_requestAction, "time");
					std::string fadeInTimeStr = this->GetQueryValue(_requestAction, "fadeintime");
					std::string fadeOutTimeStr = this->GetQueryValue(_requestAction, "fadeouttime");
					std::string maxVolumeStr = this->GetQueryValue(_requestAction, "maxvolume");
					boost::uint8_t maxVolume = 75;
					boost::uint32_t fadeOutTime = 0;

					if (!fadeInTimeStr.empty() && this->toInt(fadeInTimeStr) < 0)
						throw std::runtime_error("no correct fadeIn time duration defined!");

					// set some standard value if not defined
					if (!maxVolumeStr.empty())
						maxVolume = (boost::uint8_t)this->toInt(maxVolumeStr);
					if (!fadeOutTimeStr.empty())
						fadeOutTime = (boost::uint8_t)this->toInt(fadeOutTimeStr);

					if (maxVolume > 100)
						maxVolume = 100;
					else if (maxVolume < 0)
						maxVolume = 75;

					this->StartWakeUpTimerThread(zoneRenderer->GetUDN(), timeStr.empty() ? 0 : this->toInt(timeStr) * 1000, this->toInt(fadeInTimeStr) * 1000, fadeOutTime * 1000, maxVolume);
				}
				else if (_requestAction.action == ACTION_FADETOVOLUME)
				{
					boost::uint8_t volume = (boost::uint8_t)this->GetVolumeValueFromRequestAction(_requestAction);
					std::string fadeTimeStr = this->GetQueryValue(_requestAction, "fadetime");
					boost::uint32_t fadeTime = 0;

					if (fadeTimeStr.empty())
						throw std::runtime_error("fade time has to be defined!");
					fadeTime = (boost::uint32_t)this->toInt(fadeTimeStr);

					this->StartFadeToVolumeThread(zoneRenderer->GetUDN(), fadeTime * 1000, volume);
				}
				else if (_requestAction.action == ACTION_PLAYMODE)
				{
					std::string playModeStr = this->GetQueryValue(_requestAction, "value");
					boost::uint8_t playModeInt;

					if (playModeStr.empty())
						throw std::runtime_error("value has to be defined!");

					// get playmode integer and convert it to enum value
					// Look up enum 'MediaRendererPlayMode' for values!
					playModeInt = this->toInt(playModeStr);
					MediaRendererPlayMode playMode = (MediaRendererPlayMode)playModeInt;

					zoneRenderer->SetPlayMode(playMode, true);

				}
				else if (_requestAction.action == ACTION_LOADCONTAINER)
				{
					std::string cid = this->GetQueryValue(_requestAction, "cid");

					if (cid.empty())
						throw std::runtime_error("container id has to be defined!");

					// playlist name is unescaped, we have to escape it for playlist playing
					cid = Utils::EncodeValue(cid);

					zoneRenderer->LoadContainer(cid, -1, false);
				}
				else if (_requestAction.action == ACTION_LOADURI)
				{
					std::string uri = this->GetQueryValue(_requestAction, "uri");

					if (uri.empty())
						throw std::runtime_error("uri has to be defined!");

					// uri is http value. we do not have to encode it! we can pass it directly!

					zoneRenderer->LoadUri(uri, false);
				}
				else if (_requestAction.action == ACTION_LOADSINGLE)
				{
					std::string iid = this->GetQueryValue(_requestAction, "iid");

					if (iid.empty())
						throw std::runtime_error("iid has to be defined!");

					// uri is unescaped, we have to escape it for playlist playing
					iid = Utils::EncodeValue(iid);

					zoneRenderer->LoadSingle(iid, false);
				}
				else if (_requestAction.action == ACTION_PAUSE)
				{
					zoneRenderer->Pause(true);
				}
				else if (_requestAction.action == ACTION_TOGGLEPAUSE)
				{
					if (zoneRenderer->GetMediaRendererState().transportState == MediaRendererTransportState::MRTS_PLAYING)
						zoneRenderer->Pause(true);
					else
						zoneRenderer->Play(true);

				}
				else
				{
					throw std::runtime_error("action is not defined!");
				}


				// all commands can have the 'wait' key which defines how long the system should wait until it can handle the next request
				std::string waitAfterOperation = this->GetQueryValue(_requestAction, "wait");
				if (!waitAfterOperation.empty())
					waitTime = this->toInt(waitAfterOperation);

				if (!waitTime)
					waitTime = _requestAction.waitAfterOperation;

				if (!waitTime)
					waitTime = this->GetStandardWaitTimeForRequestAction(_requestAction);

				// now that request is done, we can unlock the managers so they can receive updates from other threads (zonChanged, deviceListChangesd...) again
				this->GetDeviceManager()->UnLock();
				this->GetZoneManager()->UnLock();

				if (waitTime)
				{
					this->Log(LogType::LOGDEBUG, "Sleeping for " + std::to_string(waitTime) + " ms", __FUNCTION__);
					this->Sleep(waitTime);
					this->Log(LogType::LOGDEBUG, "Sleeping done...", __FUNCTION__);
				}

			}
			catch(std::exception &ex)
			{
				this->GetDeviceManager()->UnLock();
				this->GetZoneManager()->UnLock();
				throw std::runtime_error(ex.what());
			}
			catch (const std::string& ex)
			{
				this->GetDeviceManager()->UnLock();
				this->GetZoneManager()->UnLock();
				throw std::runtime_error(ex);
			}
			catch (...)
			{
				this->GetDeviceManager()->UnLock();
				this->GetZoneManager()->UnLock();
				throw std::runtime_error("Unresolved Error");
			}
			
		}


		void RaumkernServer::InterruptThreadsForZone(std::string _zoneUDN)
		{			
			boost::thread* thread = nullptr;
			boost::unordered_map<std::string, boost::thread*>::iterator	 zoneThreadsIterator;

			// we may interrupt for one zone, or do interrupt for all threads
			//if (_zoneUDN.empty())
				zoneThreadsIterator = zoneThreadMap.begin();
			//else
			//	zoneThreadsIterator = zoneThreadMap.find(_zoneUDN);

			// wait for all threads to finish (we do interrupt them!)		
			for (zoneThreadsIterator; zoneThreadsIterator != zoneThreadMap.end();)
			{
				thread = zoneThreadsIterator->second;
				if (thread->joinable() && zoneThreadsIterator->first == _zoneUDN)
				{
					this->Log(LogType::LOGDEBUG, "Finishing running thread for zone " + zoneThreadsIterator->first, __FUNCTION__);
					thread->interrupt();
					thread->join();
					
					delete thread;
					zoneThreadMap.erase(zoneThreadsIterator++);
				}
				else
				{
					++zoneThreadsIterator;
				}
			}			
		}


		void RaumkernServer::StartFadeToVolumeThread(std::string _zoneUDN, boost::uint32_t _fadeTimeMS, boost::uint8_t _volume)
		{
			//INFO: here we do not need to finish the threads. Fade toos can be done inside a waiting thread for now
			this->Log(LogType::LOGDEBUG, "Starting worker thread for fade to volume...", __FUNCTION__);
			boost::thread* thread = new boost::thread(FadeToVolumeThread, _zoneUDN, _fadeTimeMS, _volume, boost::ref(*this));
			zoneThreadMap.insert(std::make_pair(_zoneUDN, thread));
		}

		void RaumkernServer::StartSleepTimerThread(std::string _zoneUDN, ::uint32_t _sleepTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS)
		{
			this->InterruptThreadsForZone(_zoneUDN);
			this->Log(LogType::LOGDEBUG, "Starting worker thread for sleep timer...", __FUNCTION__);
			boost::thread* thread = new boost::thread(SleepTimerThread, _zoneUDN, _sleepTimeDurationMS, _fadeOutTimeDurationMS, boost::ref(*this));
			zoneThreadMap.insert(std::make_pair(_zoneUDN, thread));
		}

		void RaumkernServer::StartWakeUpTimerThread(std::string _zoneUDN, ::uint32_t _wakeUpTimeDurationMS, boost::uint32_t _fadeInTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS, boost::uint8_t _maxVolume)
		{
			this->InterruptThreadsForZone(_zoneUDN);
			this->Log(LogType::LOGDEBUG, "Starting worker thread for wake up timer...", __FUNCTION__);
			boost::thread* thread = new boost::thread(WakeUpTimerThread, _zoneUDN, _wakeUpTimeDurationMS, _fadeInTimeDurationMS, _fadeOutTimeDurationMS, _maxVolume, boost::ref(*this));
			zoneThreadMap.insert(std::make_pair(_zoneUDN, thread));
		}



		void RaumkernServer::FadeToVolumeThread(std::string _zoneUDN, boost::uint32_t _fadeTimeMS, boost::uint8_t _volume, RaumkernServer &_class)
		{				
			try
			{			
				if (_fadeTimeMS > 0)
					_class.VolumeInDeCrease(_zoneUDN, _fadeTimeMS, _volume);
			}
			catch (boost::thread_interrupted const&)
			{
				// thread interrupted... 
			}
			catch (std::exception &e)
			{
				std::string exceptionText = e.what();
				_class.Log(LogType::LOGERROR, exceptionText, __FUNCTION__);
			}
		}


		void RaumkernServer::SleepTimerThread(std::string _zoneUDN, boost::uint32_t _sleepTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS, RaumkernServer &_class)
		{
			RaumfeldVirtualMediaRenderer *zoneRenderer = nullptr;;
			boost::uint8_t volumeStart = 0;
			boost::posix_time::time_duration diff;
			boost::uint32_t diffMS = 0;
			boost::posix_time::ptime timeOnStart, timeOnTest;

			try
			{

				_class.GetZoneManager()->Lock();
				_class.GetDeviceManager()->Lock();

				zoneRenderer = _class.GetZoneManager()->GetZoneRenderer(_zoneUDN);		
				if (zoneRenderer)
				{
					volumeStart = zoneRenderer->GetMediaRendererState().volume;
					timeOnStart = boost::posix_time::second_clock::local_time();
				}

				_class.GetZoneManager()->UnLock();
				_class.GetDeviceManager()->UnLock();

				if (!zoneRenderer)
					return;				

				// wait now for the sleep duration we got in a while loop to be sure we can kill this thread if necessary
				while (diffMS < _sleepTimeDurationMS)
				{
					_class.Sleep(500);

					boost::this_thread::interruption_point();
	
					timeOnTest = boost::posix_time::second_clock::local_time();
					diff = timeOnTest - timeOnStart;
					diffMS = (boost::uint32_t)diff.total_milliseconds();
				}
										

				if (_fadeOutTimeDurationMS > 0)
					_class.VolumeInDeCrease(_zoneUDN, _fadeOutTimeDurationMS, 0);


				_class.GetZoneManager()->Lock();
				_class.GetDeviceManager()->Lock();

				zoneRenderer = _class.GetZoneManager()->GetZoneRenderer(_zoneUDN);
				if (zoneRenderer)
				{
					zoneRenderer->Stop(true);

					// wait till rendere is really stopped, otherwise last couple of ms may be to loud again 
					while (zoneRenderer->GetMediaRendererState().transportState == MediaRendererTransportState::MRTS_PLAYING || zoneRenderer->GetMediaRendererState().transportState == MediaRendererTransportState::MRTS_TRANSITIONING)
					{
						boost::this_thread::interruption_point();
						_class.Sleep(50);
					}
					// only to be on the safe side...
					_class.Sleep(100);

					// set back old volume
					zoneRenderer->SetVolume(volumeStart);
				}

				_class.GetZoneManager()->UnLock();
				_class.GetDeviceManager()->UnLock();
											
			}
			catch (boost::thread_interrupted const&)
			{
				// thread interrupted... 
			}
			catch (std::exception &e)
			{
				std::string exceptionText = e.what();
				_class.Log(LogType::LOGERROR, exceptionText, __FUNCTION__);
			}
		}


		void RaumkernServer::WakeUpTimerThread(std::string _zoneUDN, boost::uint32_t _wakeUpTimeDurationMS, boost::uint32_t _fadeInTimeDurationMS, boost::uint32_t _fadeOutTimeDurationMS, boost::uint8_t _maxVolume, RaumkernServer &_class)
		{
			RaumfeldVirtualMediaRenderer *zoneRenderer = nullptr;
			boost::uint8_t volumeStart = 0;
			boost::posix_time::time_duration diff;
			boost::uint32_t diffMS = 0;
			boost::posix_time::ptime timeOnStart, timeOnTest;

			try
			{

				_class.GetZoneManager()->Lock();
				_class.GetDeviceManager()->Lock();

				zoneRenderer = _class.GetZoneManager()->GetZoneRenderer(_zoneUDN);
				if (zoneRenderer)
				{
					volumeStart = zoneRenderer->GetMediaRendererState().volume;
					// we have to begin from value 0					
					zoneRenderer->SetVolume(0, true);
					// Set volume on each room renderer! Because zone volume isn't a real set to value at all. it will do something like change
					for (auto roomInformation : _class.GetZoneManager()->GetZoneInformation(_zoneUDN).roomsUDN)
					{
						zoneRenderer->SetRoomVolume(roomInformation, 0, true);
					}					

					_class.Sleep(100);

					// if not already playing. then play
					if (zoneRenderer->GetMediaRendererState().transportState != MediaRendererTransportState::MRTS_PLAYING)
						zoneRenderer->Play(true);

					if (_fadeInTimeDurationMS <= 0)
						zoneRenderer->SetVolume(_maxVolume);
				}
				
				_class.GetZoneManager()->UnLock();
				_class.GetDeviceManager()->UnLock();

				if (!zoneRenderer)
					return;
			

				boost::this_thread::interruption_point();

				_class.Sleep(500);

				// do fade in of music ("VolumeInDeCrease" will care of locking and unlocking zone/device mutex)
				if (_fadeInTimeDurationMS > 0)
					_class.VolumeInDeCrease(_zoneUDN, _fadeInTimeDurationMS, _maxVolume);


				// wait now for the _wakeUpTimeDurationMS we got in a while loop to be sure we can kill this thread if necessary
				// if we got no time well not stop and play forever until user stops or reboot of rf system will be done
				if (_wakeUpTimeDurationMS)
				{

					timeOnStart = boost::posix_time::second_clock::local_time();

					while (diffMS < _wakeUpTimeDurationMS)
					{
						_class.Sleep(500);

						boost::this_thread::interruption_point();

						timeOnTest = boost::posix_time::second_clock::local_time();
						diff = timeOnTest - timeOnStart;
						diffMS = (boost::uint32_t)diff.total_milliseconds();
					}

					// fade out on end
					if (_fadeOutTimeDurationMS > 0)
						_class.VolumeInDeCrease(_zoneUDN, _fadeOutTimeDurationMS, 0);

					_class.GetZoneManager()->Lock();
					_class.GetDeviceManager()->Lock();
					
					// check if zone is still present. if not.. we can not do this action anymore, so we have to quit
					zoneRenderer = _class.GetZoneManager()->GetZoneRenderer(_zoneUDN);
					if (zoneRenderer)
					{						
						zoneRenderer->Stop(true);
						// wait till renderer is really stopped, otherwise last couple of ms may be to loud again 
						while (zoneRenderer->GetMediaRendererState().transportState == MediaRendererTransportState::MRTS_PLAYING || zoneRenderer->GetMediaRendererState().transportState == MediaRendererTransportState::MRTS_TRANSITIONING)
						{
							boost::this_thread::interruption_point();
							_class.Sleep(20);
						}
						// only to be on the safe side...
						_class.Sleep(100);

						zoneRenderer->SetVolume(_maxVolume);
					}

					_class.GetZoneManager()->UnLock();
					_class.GetDeviceManager()->UnLock();
							
				}
			
			}
			catch (boost::thread_interrupted const&)
			{
				// thread interrupted... 
			}
		}


		// RaumkernServer::VolumeInDeCrease
		// has always to be called from a thread thread! Will increase or decrease the volume from the actual one to the volume given in_'toVolume'
		void RaumkernServer::VolumeInDeCrease(std::string _zoneUDN, boost::uint32_t _timeToDoMS, boost::uint8_t _toVolume)
		{
			RaumfeldVirtualMediaRenderer *zoneRenderer = nullptr;
			boost::uint8_t volumeStart = 0, volumeNow = 0, volumeDiff = 0;
			boost::uint32_t waitTime, waitTimeRest, waitTimeLoopCount, loopIdx;
			boost::int8_t volumeChangeValue = 1;

			this->GetZoneManager()->Lock();
			this->GetDeviceManager()->Lock();

			zoneRenderer = this->GetZoneManager()->GetZoneRenderer(_zoneUDN);
			if (zoneRenderer)
			{
				volumeStart = zoneRenderer->GetMediaRendererState().volume;
				volumeNow = volumeStart;
				volumeDiff = abs(volumeStart - _toVolume);
			}

			this->GetZoneManager()->UnLock();
			this->GetDeviceManager()->UnLock();

			if (!zoneRenderer)
				return;

			if (volumeDiff == 0)
				return;

			if (volumeStart > _toVolume)
				volumeChangeValue = -1;

			waitTime = (boost::uint32_t)(_timeToDoMS / volumeDiff);

			waitTimeLoopCount = (boost::uint32_t)((double)waitTime / 10.0);
			waitTimeRest = waitTime - (10 * waitTimeLoopCount);

			while (volumeNow != _toVolume)
			{		
				
				// wo do this split beacuse of killing the thread (okay i should investigate the threads a little bit more and use thread innterupts / thread waits shuld handle this)
				for (loopIdx = 0; loopIdx < waitTimeLoopCount; loopIdx++)
				{
					this->Sleep(10);

					boost::this_thread::interruption_point();
				}
				this->Sleep(waitTimeRest);

				boost::this_thread::interruption_point();

				this->GetZoneManager()->Lock();
				this->GetDeviceManager()->Lock();

				zoneRenderer = this->GetZoneManager()->GetZoneRenderer(_zoneUDN);
				if (zoneRenderer)
				{
					volumeNow += volumeChangeValue;
					zoneRenderer->SetVolume(volumeNow);
				}

				this->GetZoneManager()->UnLock();
				this->GetDeviceManager()->UnLock();
			}

			
		}


		boost::int16_t RaumkernServer::GetVolumeValueFromRequestAction(RequestAction _requestAction)
		{
			std::string volumeValue = this->GetQueryValue(_requestAction, "value");
			if (volumeValue.empty())
				throw std::runtime_error("no value defined!");

			boost::int32_t volume = this->toInt(volumeValue);
			if (volume > 100 || volume < -100)
				throw std::runtime_error("value has to be between -100 and 100!");

			return (boost::int16_t)volume;
		}


		void RaumkernServer::InitStandardRequestwaitTimeMap()
		{	
			requestWaitMap[ACTION_CREATEZONE] = 3000;
			requestWaitMap[ACTION_DROPFROMZONE] = 3000;
			requestWaitMap[ACTION_ADDTOZONE] = 3000;
			requestWaitMap[ACTION_LOADPLAYLIST] = 5000;
			requestWaitMap[ACTION_LOADSINGLE] = 500;
			requestWaitMap[ACTION_LOADURI] = 500;
		}

		boost::uint32_t RaumkernServer::GetStandardWaitTimeForRequestAction(RequestAction _requestAction)
		{			
			boost::unordered_map<std::string, boost::int32_t>::iterator it;						

			it = requestWaitMap.find(_requestAction.action);
			if (it != requestWaitMap.end())
				return it->second;						
			return ACTION_STANDARDWAITTIME;
		}


		RequestActionScope RaumkernServer::GetScopeForAction(std::string _action, RequestActionScope _preferedScope)
		{
			std::vector<std::string> exclusiveRoomActions = { ACTION_CREATEZONE, ACTION_DROPFROMZONE, ACTION_ADDTOZONE };
			std::vector<std::string> exclusiveZoneActions = { ACTION_PLAYNEXT, ACTION_PLAYPREV, ACTION_PLAY, ACTION_LOADPLAYLIST, ACTION_SLEEPTIMER, ACTION_WAKEUPTIMER, ACTION_FADETOVOLUME, ACTION_LOADCONTAINER, ACTION_LOADURI, ACTION_LOADSINGLE, ACTION_PAUSE, ACTION_TOGGLEPAUSE, ACTION_PLAYMODE };

			boost::algorithm::to_lower(_action);

			if (std::find(exclusiveRoomActions.begin(), exclusiveRoomActions.end(), _action) != exclusiveRoomActions.end())
				return RequestActionScope::RAS_ROOM;

			if (std::find(exclusiveZoneActions.begin(), exclusiveZoneActions.end(), _action) != exclusiveZoneActions.end())
				return RequestActionScope::RAS_ZONE;

			return _preferedScope;
		}

		bool RaumkernServer::VirtualRendererNeededForAction(std::string _action, RequestActionScope _preferedScope)
		{
			std::vector<std::string> actionsWhichNeedsZone = { ACTION_VOLUME, ACTION_MUTE, ACTION_UNMUTE, ACTION_FADETOVOLUME, ACTION_VOLUMECHANGE, ACTION_PAUSE, ACTION_PLAYNEXT, ACTION_PLAYPREV, ACTION_PLAY, ACTION_LOADPLAYLIST, ACTION_TOGGLEMUTE, ACTION_SLEEPTIMER, ACTION_WAKEUPTIMER, ACTION_LOADCONTAINER, ACTION_LOADURI, ACTION_LOADSINGLE, ACTION_TOGGLEPAUSE, ACTION_PLAYMODE };
			RequestActionScope actionScope = RaumkernServer::GetScopeForAction(_action, _preferedScope);
			if(actionScope == RequestActionScope::RAS_ZONE)
				return true;
		
			if (std::find(actionsWhichNeedsZone.begin(), actionsWhichNeedsZone.end(), _action) != actionsWhichNeedsZone.end())
				return true;

			return false;
		}


		void RaumkernServer::HandleRoomRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request)
		{
			this->Log(LogType::LOGDEBUG, "handle room request '" + _request->method + " " + _request->path + "'", __FUNCTION__);
			
			try
			{
				this->DoRequestAction(this->GetRequestActionFromPath(_request->path));
				std::string content = "Request aknowledged";
				_response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
			}
			catch (std::exception &e)
			{				 				

				std::string exceptionText = e.what();
				std::string content = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, content, __FUNCTION__);
				_response << "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
			}
		}


		void RaumkernServer::HandleZoneRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request)
		{
			this->Log(LogType::LOGDEBUG, "handle zone request '" + _request->method + " " + _request->path + "'", __FUNCTION__);

			try
			{
				this->DoRequestAction(this->GetRequestActionFromPath(_request->path));
				std::string content = "Request aknowledged";
				_response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
			}
			catch (std::exception &e)
			{
				std::string exceptionText = e.what();
				std::string content = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, content, __FUNCTION__);
				_response << "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
			}
		}

		void RaumkernServer::HandlePresetRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request)
		{
			this->Log(LogType::LOGDEBUG, "handle preset request '" + _request->method + " " + _request->path + "'", __FUNCTION__);

			try
			{
				this->DoRequestPreset(this->GetRequestPresetFromPath(_request->path));
				std::string content = "Request aknowledged";
				_response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
			}
			catch (std::exception &e)
			{
				std::string exceptionText = e.what();
				std::string content = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, content, __FUNCTION__);
				_response << "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
			}
		}


		void RaumkernServer::HandleBadRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request)
		{
			this->Log(LogType::LOGDEBUG, "bad request '" + _request->method + " " + _request->path + "'", __FUNCTION__);
			std::string content = "Not suitable path for RaumServer! Please lookup readme.html for correct request path/syntax";
			_response << "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
		}


		void RaumkernServer::HandleWebClientRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request)
		{
			std::string exceptionText = "";
			std::vector<char> content;
			std::string error;

			this->Log(LogType::LOGDEBUG, "Web client request '" + _request->method + " " + _request->path + "'", __FUNCTION__);

			try
			{
				HTMLResponseHandler responseHandler;
				ResponseHandlerResponseReturn responseResponseHandler;

				boost::uint16_t len;
				_request->content.read((char*)&len, 2);
				std::string contentString(len, '\0');
				_request->content.read(&contentString[0], len);
				
				responseResponseHandler = responseHandler.CreateResponseForRequest(_request->path, _request->header, contentString);
				content = responseResponseHandler.binaryContent;

				//_response << "HTTP/1.1 200 OK\r\nContent-Length: " << content.length() << "\r\n" + Utils::CreateHTTPHeaderVars(responseResponseHandler.header) + "\r\n" << content;
				_response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.size() << "\r\n" + Utils::CreateHTTPHeaderVars(responseResponseHandler.header) + "\r\n";
				_response.write(reinterpret_cast<const char*>(content.data()), content.size());
				return;
			}
			catch (std::exception &e)
			{
				exceptionText = e.what();
				error = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, error, __FUNCTION__);
			}
			catch (...)
			{
				error = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, error, __FUNCTION__);
			}

			_response << "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << error.length() << "\r\n\r\n" << error;
		}

		void RaumkernServer::HandleJSONRequests(std::ostream& _response, std::shared_ptr<ServerBase<boost::asio::ip::tcp::socket>::Request> _request)
		{			
			std::string exceptionText = "";
			std::string content = "";

			this->Log(LogType::LOGDEBUG, "JSON request '" + _request->method + " " + _request->path + "'", __FUNCTION__);
			
			try
			{				
				ResponseHandlerResponseReturn responseResponseHandler;

				boost::uint16_t len;
				_request->content.read((char*)&len, 2);
				std::string contentString(len, '\0');
				_request->content.read(&contentString[0], len);
				
				responseResponseHandler = jsonResponseHandler.CreateResponseForRequest(_request->path, _request->header, contentString);
				content = responseResponseHandler.content;	

				_response << "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n" + Utils::CreateHTTPHeaderVars(responseResponseHandler.header) +  "\r\n" << content;
				return;
			}
			catch (std::exception &e)
			{
				exceptionText = e.what();
				content = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, content, __FUNCTION__);				
			}
			catch (...)
			{
				content = exceptionText + "(" + _request->path + ")";
				this->Log(LogType::LOGERROR, content, __FUNCTION__);
			}
		
			_response << "HTTP/1.1 400 Bad Request\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: " << content.length() << "\r\n\r\n" << content;
		}

		bool RaumkernServer::StartServer()
		{
			if (raumkernSettings.RAUMKERNEL_SERVER_PORT.empty())
			{
				this->Log(LogType::LOGERROR, "Port for RaumServer not defined! Please Check settings.xml!", __FUNCTION__);
				signalWebserverStartFailed("Port for RaumServer not defined! Please Check settings.xml!");
				return false;
			}
			
			try
			{				
				startServerThread = boost::thread(&RaumkernServer::StartServerThread, this);
			}
			catch (std::exception &e)
			{
				std::string exceptionText = e.what();
				this->Log(LogType::LOGERROR, exceptionText, __FUNCTION__);	
				signalWebserverStartFailed(exceptionText);
				return false;
			}

			return true;
		}

		void RaumkernServer::StartServerThread()
		{
			bool serverStartedSignaled = false;
			bool tryAgain = true;

			while (tryAgain)
			{
				try
				{
					this->Log(LogType::LOGINFO, "Starting RaumServer with port '" + raumkernSettings.RAUMKERNEL_SERVER_PORT + "' ...", __FUNCTION__);

					// creating server on port given in options. we only use one thread. no need for more
					server = new Server<HTTP>(this->toInt(raumkernSettings.RAUMKERNEL_SERVER_PORT), raumkernSettings.RAUMKERNEL_SERVER_MAXALLOWEDTHREADS, raumkernSettings.RAUMKERNEL_SERVER_TIMOUTREQUEST_SEC, raumkernSettings.RAUMKERNEL_SERVER_TIMOUTCONTENT_SEC);

					// add uri path's we have to handle (do this for GET and POST action)	
					// eg.: http://10.0.0.4/raumserver/room/Schlafzimmer/createZone			
					// eg.: http://10.0.0.4/raumserver/room/Schlafzimmer/loadPlaylist?list=Test
					// eg.: http://10.0.0.4/raumserver/room/Schlafzimmer/play?track=1			
					server->resource["^/raumserver/room/(.*)/(.*)"]["POST"] = std::bind(&RaumkernServer::HandleRoomRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->resource["^/raumserver/room/(.*)/(.*)"]["GET"] = std::bind(&RaumkernServer::HandleRoomRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->resource["^/raumserver/zone/(.*)/(.*)"]["POST"] = std::bind(&RaumkernServer::HandleZoneRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->resource["^/raumserver/zone/(.*)/(.*)"]["GET"] = std::bind(&RaumkernServer::HandleZoneRequests, this, std::placeholders::_1, std::placeholders::_2);

					// add resource for /raumserver/preset/idofpreset/run?id1=test&id2=test2
					server->resource["^/raumserver/preset/(.*)/(.*)"]["GET"] = std::bind(&RaumkernServer::HandlePresetRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->resource["^/raumserver/preset/(.*)/(.*)"]["POST"] = std::bind(&RaumkernServer::HandlePresetRequests, this, std::placeholders::_1, std::placeholders::_2);

					// add resource for json GET and POST requests
					server->resource["^/raumserver/json/(.*)"]["GET"] = std::bind(&RaumkernServer::HandleJSONRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->resource["^/raumserver/json/(.*)"]["POST"] = std::bind(&RaumkernServer::HandleJSONRequests, this, std::placeholders::_1, std::placeholders::_2);

					// add resource for WebClient HTML
					server->resource["^/raumserver/webclient/(.*)"]["GET"] = std::bind(&RaumkernServer::HandleWebClientRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->resource["^/raumserver/webclient/(.*)"]["POST"] = std::bind(&RaumkernServer::HandleWebClientRequests, this, std::placeholders::_1, std::placeholders::_2);

					// add bad requests
					server->default_resource["^/?(.*)$"]["GET"] = std::bind(&RaumkernServer::HandleBadRequests, this, std::placeholders::_1, std::placeholders::_2);
					server->default_resource["^/?(.*)$"]["POST"] = std::bind(&RaumkernServer::HandleBadRequests, this, std::placeholders::_1, std::placeholders::_2);


					// we will asume that start will be ok, if not then the webserverShutdown signal will be thrownin thread
					this->signalWebserverReady();
					serverStartedSignaled = true;

					// starting server will hang in here until server is closed
					this->GetServerObject()->start();
					tryAgain = false;

				}
				catch (std::exception &e)
				{
					std::string exceptionText = e.what();
					this->Log(LogType::LOGERROR, exceptionText, __FUNCTION__);
					if (serverStartedSignaled)
						this->signalWebserverShutdown();

					// wevserver throwed an error, we try to start him (again) after a defined wait time
					tryAgain = true;
					this->Sleep(2000);

				}
				catch (boost::thread_interrupted const&)
				{
					tryAgain = false;
					// thread interrupted... 
				}
			}

			this->Log(LogType::LOGINFO, "RaumServer thread closing...", __FUNCTION__);
		}


		void RaumkernServer::StopServer()
		{	
			if (server)
			{
				// cancel all open long running requests	
				jsonResponseHandler.SetCancelRequets(true);

				server->stop();

				if (startServerThread.joinable())
				{
					startServerThread.interrupt();
					startServerThread.join();
				}

				// okay... this is a tryout for linux vecause on linux port seems to be already in use from the old instance if we go to fast
				this->Sleep(1000);

				signalWebserverShutdown();
			}
		}

		bool RaumkernServer::LoadRequestPresets(bool _force)
		{
			if (!_force && requestPresetsMap.size() > 0)
				return true;

			this->Log(LogType::LOGDEBUG, "Load request presets from file", __FUNCTION__);

			if (raumkernSettings.RAUMKERNEL_SERVER_REQUESTPRESETFILEPATH.empty())
			{
				this->Log(LogType::LOGDEBUG, "No request presets file specified!", __FUNCTION__);
				return false;
			}

			std::ifstream presetsFileStream(raumkernSettings.RAUMKERNEL_SERVER_REQUESTPRESETFILEPATH.c_str());
			if (presetsFileStream.fail())
			{
				this->Log(LogType::LOGERROR, "Cannot open preset file '" + raumkernSettings.RAUMKERNEL_SERVER_REQUESTPRESETFILEPATH + "'", __FUNCTION__);
				return false;
			}

			std::vector<char> buffer((std::istreambuf_iterator<char>(presetsFileStream)), std::istreambuf_iterator<char>());
			buffer.push_back('\0');
			std::string xml(buffer.begin(), buffer.end());

			return this->LoadRequestPresetsFromXML(xml);
		}


		bool RaumkernServer::LoadRequestPresetsFromXML(std::string _xml)
		{		
			xml_document<> doc;
			std::string presetId;
			xml_node<> *presetsNode, *presetNode, *requestNode;
			xml_attribute<> *attribute;

			forceLoadPresets = false;

			requestPresetsMap.clear();

			// to parse the string we have to put it ino char* (because c_str() returns const char*)
			char* cstr = new char[_xml.size() + 1];
			strcpy(cstr, _xml.c_str());
			doc.parse<0>(cstr);				
		
			presetsNode = doc.first_node("presets", 0, false);
			if (!presetsNode)
			{
				this->Log(LogType::LOGERROR, "Request presets XML  does not contain presets block!", __FUNCTION__);
				return false;
			}


			presetNode = presetsNode->first_node("preset", 0, false);
			while (presetNode)
			{
				attribute = presetNode->first_attribute("id");
				if (attribute)
				{
					presetId = attribute->value();
					if (!presetId.empty())
					{						
						std::vector<RequestAction> requestActionList;

						boost::algorithm::to_lower(presetId);

						requestNode = presetNode->first_node("request", 0, false);
						while (requestNode)
						{
							RequestAction requestAction;
							std::string path = requestNode->value();
							boost::uint32_t waitMS = 0;

							attribute = requestNode->first_attribute("waitAfterOperationMS");
							if (attribute)
								waitMS = this->toInt(attribute->value());
							
							if (!path.empty())
							{			
								requestAction = this->GetRequestActionFromPath(path);
								requestAction.waitAfterOperation = waitMS;
								requestActionList.push_back(requestAction);

								// room is not present! we have to force reading again on next try
								if (requestAction.roomUDN.empty() || requestAction.roomName.empty())
									forceLoadPresets = true;
							}

							requestNode = requestNode->next_sibling("request", 0, false);
						}
				
						requestPresetsMap.insert(std::make_pair(presetId, requestActionList));
					}
					else
					{
						this->Log(LogType::LOGERROR, "Preset does not have id specified!", __FUNCTION__);
					}					
				}
				else
				{
					this->Log(LogType::LOGERROR, "Preset does not have id specified!", __FUNCTION__);
				}							
												
				presetNode = presetNode->next_sibling("preset", 0, false);
			}


			return true;
		}

		void RaumkernServer::DoRequestPreset(RequestPreset _requestPreset)
		{
			if (_requestPreset.path.empty())
				throw std::runtime_error("'path' for preset request not correct!");
			if (_requestPreset.presetId.empty())
				throw std::runtime_error("preset id not present!");
			if (_requestPreset.action.empty())
				throw std::runtime_error("no action defined!");
			
			if (!this->LoadRequestPresets(forceLoadPresets))
				throw std::runtime_error("loading of request presets failed!");

			boost::unordered_map<std::string, std::vector<RequestAction>>::iterator	presetIt;

			// checck if there is a preset id which we can handle, if not we have send some info back that we have not found a proper preset
			presetIt = requestPresetsMap.find(_requestPreset.presetId);
			if (presetIt == requestPresetsMap.end())
				throw std::runtime_error("preset with id '" + _requestPreset.presetId + "' not found!");


			if (_requestPreset.action == "run")
			{				
			
				// we can only handle one preset after the other, no parallel preset doing is allowed!
				// so to insist only one preset is running we du this mith a mutex lock
				boost::mutex::scoped_lock scoped_lock(presetRunningLock);

				// befoe we can start a request we have to kill all requests which may be affected by the nre request
				// so we kill all zone requests from the zones where we may do some action on it
				std::vector<RequestAction> requestList = presetIt->second;
				for (auto &requestAction : requestList)
				{
					ZoneInformation zoneInfo = this->GetZoneManager()->GetZoneInformationFromRoom(requestAction.roomUDN);
					this->InterruptThreadsForZone(zoneInfo.UDN);
				}

				// a preset consists of a list of request. We do now process this requests
				requestList = presetIt->second;
				for (auto &requestAction : requestList)
				{
					// replace placeholders url of requestAction, not defined placeholders will be deleted
					this->ReplaceRequestPlaceholders(requestAction, _requestPreset, true);			
					this->DoRequestAction(requestAction);
				}

				// after preset was run, we force some sleep!
				this->Sleep(500);

			}
			else
			{
				throw std::runtime_error("action is not defined!");
			}

		}

		void RaumkernServer::ReplaceRequestPlaceholders(RequestAction &_requestAction, RequestPreset _requestPreset, bool _removeNotFound)
		{		
			boost::unordered_map<std::string, std::string>::iterator it;

			for (auto &mapValue : _requestAction.queryValues)
			{
				std::string value = mapValue.second;
				if (!value.empty())
				{
					if (value.front() == '[' && value.back() == ']')
					{
						std::string key = value.substr(1, value.length() - 2);
						boost::algorithm::to_lower(key);
						it = _requestPreset.queryValues.find(key);
						if (it != _requestPreset.queryValues.end())
						{
							mapValue.second = it->second;
						}
						else
						{
							if (_removeNotFound)
								mapValue.second = "";
						}
					}
				}
			}
		}

		void RaumkernServer::RemoveRequestPlaceholders(RequestAction &_requestAction)
		{
			for (auto &mapValue : _requestAction.queryValues)
			{
				std::string value = mapValue.second;
				if (value.front() == '[' && value.back() == ']')
				{
					mapValue.second = "";
				}
			}
		}


	}
}




