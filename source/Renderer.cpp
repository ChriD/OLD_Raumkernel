#include "Renderer.h"
//#include "Raumkern.h"
#include "DeviceManager.h"
#include "MediaServer.h"
#include "ContentManager.h"
#include "urlcpp/Url.h"
#include <boost/network/protocol/http/client.hpp>

namespace Raumkernel
{
	
		MediaRenderer::MediaRenderer() : UPNPDevice()
		{			
			avTransportProxy = nullptr;
			renderingControlProxy = nullptr;
			connectionManagerProxy = nullptr;
		}

		MediaRenderer::~MediaRenderer()
		{
			// no need to remove subscriptions! delete of proxies will handle this correct!
			this->DeleteProxies();
		}


		bool MediaRenderer::IsVirtualRenderer()
		{
			return false;
		}

		bool MediaRenderer::IsRaumfeldRenderer()
		{
			return false;
		}

		MediaRendererState MediaRenderer::GetMediaRendererState()
		{
			return mediaRendererState;
		}

		
		void MediaRenderer::CreateProxies(bool _allowCreate)
		{			
		}

		void MediaRenderer::RemoveSubscriptions()
		{
		}

		void MediaRenderer::AddSubscriptions()
		{
		}

		std::string MediaRenderer::MediaRendererPlayModeToString(MediaRendererPlayMode _playMode)
		{
			switch (_playMode)
			{
			case MediaRendererPlayMode::MRPLAYMODE_NORMAL:
				return "NORMAL";
			case MediaRendererPlayMode::MRPLAYMODE_SHUFFLE:
				return "SHUFFLE";
			case MediaRendererPlayMode::MRPLAYMODE_RANDOM:
				return "RANDOM";
			case MediaRendererPlayMode::MRPLAYMODE_REPEAT_ONE:
				return "REPEAT_ONE";
			case MediaRendererPlayMode::MRPLAYMODE_REPEAT_ALL:
				return "REPEAT_ALL";
			case MediaRendererPlayMode::MRPLAYMODE_DIRECT_1:
				return "DIRECT_1";
			case MediaRendererPlayMode::MRPLAYMODE_INTRO:
				return "INTRO";
			}
			return "NORMAL";
		}

		MediaRendererPlayMode MediaRenderer::StringToMediaRendererPlayMode(std::string _playModeString)
		{
			if (_playModeString == "NORMAL")
				return MediaRendererPlayMode::MRPLAYMODE_NORMAL;
			if (_playModeString == "SHUFFLE")
				return MediaRendererPlayMode::MRPLAYMODE_SHUFFLE;
			if (_playModeString == "RANDOM")
				return MediaRendererPlayMode::MRPLAYMODE_RANDOM;
			if (_playModeString == "REPEAT_ONE")
				return MediaRendererPlayMode::MRPLAYMODE_REPEAT_ONE;
			if (_playModeString == "REPEAT_ALL")
				return MediaRendererPlayMode::MRPLAYMODE_REPEAT_ALL;
			if (_playModeString == "DIRECT_1")
				return MediaRendererPlayMode::MRPLAYMODE_DIRECT_1;
			if (_playModeString == "INTRO")
				return MediaRendererPlayMode::MRPLAYMODE_INTRO;
			return MediaRendererPlayMode::MRPLAYMODE_NORMAL;
		}

		MediaRendererTransportState MediaRenderer::StringToMediaRendererTransportState(std::string _transportState)
		{
			if (_transportState == "PLAYING")
				return MediaRendererTransportState::MRTS_PLAYING;
			if (_transportState == "STOPPED")
				return MediaRendererTransportState::MRTS_STOPPED;
			if (_transportState == "TRANSITIONING")
				return MediaRendererTransportState::MRTS_TRANSITIONING;
			return MediaRendererTransportState::MRTS_STOPPED;
		}

		std::string MediaRenderer::MediaRendererTransportStateToString(MediaRendererTransportState _transportState)
		{	
			if (_transportState == MediaRendererTransportState::MRTS_PLAYING)
				return "PLAYING";
			if (_transportState == MediaRendererTransportState::MRTS_STOPPED)
				return "STOPPED";
			if (_transportState == MediaRendererTransportState::MRTS_TRANSITIONING)
				return "TRANSITIONING";			
			return "STOPPED";
		}

		std::string MediaRenderer::MediaRendererMuteStateToString(MediaRendererMuteState _muteState)
		{
			if (_muteState == MediaRendererMuteState::MRPMUTE_ALL)
				return "ALL";
			if (_muteState == MediaRendererMuteState::MRPMUTE_NONE)
				return "NONE";
			if (_muteState == MediaRendererMuteState::MRPMUTE_PARTIAL)
				return "PARTIAL";
			return "NONE";
		}

	

		// MediaRenderer::SubscribeSignalMediaRendererStateChanged
		void MediaRenderer::SubscribeSignalMediaRendererStateChanged(const typeSignalMediaRendererStateChanged::slot_type &_subscriber)
		{
			signalMediaRendererStateChanged.connect(_subscriber);
		}


		void MediaRenderer::DeleteProxies()
		{	
			//boost::mutex::scoped_lock scoped_lock(proxyObjects);

			if (avTransportProxy)
			{
				delete avTransportProxy;
				avTransportProxy = nullptr;
			}

			if (connectionManagerProxy)
			{
				delete connectionManagerProxy;
				connectionManagerProxy = nullptr;
			}

			if (renderingControlProxy)
			{
				delete renderingControlProxy;
				renderingControlProxy = nullptr;
			}
		}


		bool MediaRenderer::RenderingProxyAvailable(bool _allowCreate)
		{
			if (!cpDevice)
				return false;

			// if the rendere list is locked for update, we can not do any axtion on the Media Renderer
			// TODO: GetDeviceManager mit GLOBAL!!!!!
			//if (this->GetDeviceManager()->IsRendererServerListLockedForUpdate())
			//	return false;

			this->CreateProxies(_allowCreate);
			if (renderingControlProxy == nullptr)
			{
				this->Log(LogType::LOGWARNING, "Proxy Instance not available!" + this->GetIdentificationString(), __FUNCTION__);
				return false;
			}
			return true;
		}

		bool MediaRenderer::ConnectionManagerProxyAvailable(bool _allowCreate)
		{
			if (!cpDevice)
				return false;

			this->CreateProxies(_allowCreate);
			if (renderingControlProxy == nullptr)
			{
				this->Log(LogType::LOGWARNING, "Proxy Instance not available!" + this->GetIdentificationString(), __FUNCTION__);
				return false;
			}
			return true;
		}

		bool MediaRenderer::AvTransportProxyAvailable(bool _allowCreate)
		{
			if (!cpDevice)
				return false;

			this->CreateProxies(_allowCreate);
			if (renderingControlProxy == nullptr)
			{
				this->Log(LogType::LOGWARNING, "Proxy Instance not available!" + this->GetIdentificationString(), __FUNCTION__);
				return false;
			}
			return true;
		}


		// ##########################################################################

		RaumfeldMediaRenderer::RaumfeldMediaRenderer() : MediaRenderer()
		{
		}

		RaumfeldMediaRenderer::~RaumfeldMediaRenderer()
		{
		}

		void RaumfeldMediaRenderer::CreateProxies(bool _allowCreate)
		{
		}

		bool RaumfeldMediaRenderer::IsRaumfeldRenderer()
		{
			return true;
		}

		bool RaumfeldMediaRenderer::IsVirtualRenderer()
		{
			return false;
		}




		// ##########################################################################

		RaumfeldVirtualMediaRenderer::RaumfeldVirtualMediaRenderer() : RaumfeldMediaRenderer()
		{
		}

		RaumfeldVirtualMediaRenderer::~RaumfeldVirtualMediaRenderer()
		{
			// no need to remove subscriptions! delete of proxies will handle this!		
			this->DeleteProxies();
		}


		bool RaumfeldVirtualMediaRenderer::IsVirtualRenderer()
		{
			return true;
		}

		bool RaumfeldVirtualMediaRenderer::IsRaumfeldRenderer()
		{
			return true;
		}


		CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp* RaumfeldVirtualMediaRenderer::GetAvTransportProxy(bool _allowCreate)
		{
			this->CreateProxies(_allowCreate);
			return (CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp*)avTransportProxy; 
		}

		CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp* RaumfeldVirtualMediaRenderer::GetConnectionManagerProxy(bool _allowCreate)
		{
			this->CreateProxies(_allowCreate);
			return (CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp*)connectionManagerProxy;
		}

		CpProxyUpnpOrgRenderingControl_RaumfeldVirtual1Cpp* RaumfeldVirtualMediaRenderer::GetRenderingControlProxy(bool _allowCreate)
		{
			this->CreateProxies(_allowCreate);
			return (CpProxyUpnpOrgRenderingControl_RaumfeldVirtual1Cpp*)renderingControlProxy;
		}



		void RaumfeldVirtualMediaRenderer::CreateProxies(bool _allowCreate)
		{
			

			if (!cpDevice)
			{
				this->Log(LogType::LOGWARNING, "Calling 'CreateProxies' on renderer '" + this->GetIdentificationString() + "' without having CP-Device!", __FUNCTION__);
				this->DeleteProxies();

				return;
			}

			if (!_allowCreate)
				return;


			if (!avTransportProxy || !renderingControlProxy || !connectionManagerProxy)
			{
				this->Log(LogType::LOGDEBUG, "Create proxies for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);
			}

			//boost::mutex::scoped_lock scoped_lock(proxyObjects);

			if (!avTransportProxy)			avTransportProxy		= new CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp(*cpDevice);
			if (!renderingControlProxy)		renderingControlProxy	= new CpProxyUpnpOrgRenderingControl_RaumfeldVirtual1Cpp(*cpDevice);
			if (!connectionManagerProxy)	connectionManagerProxy	= new CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp(*cpDevice);
			// INFO: Add more proxies here if necesarry
		}


		void RaumfeldVirtualMediaRenderer::Play(bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;
			std::string	playSpeed = "1";

			this->Log(LogType::LOGDEBUG, "Action 'play' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{
				if (sync)
					this->GetAvTransportProxy()->SyncPlay(instance, playSpeed);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnPlay);
					this->GetAvTransportProxy()->BeginPlay(instance, playSpeed, functorAsync);
				}		
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnPlay(IAsync& _aAsync)
		{	
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndPlay(_aAsync);
			}
		}
	

		void RaumfeldVirtualMediaRenderer::Stop(bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'stop' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);
			
			try
			{				

				if (sync)
					this->GetAvTransportProxy()->SyncStop(instance);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnStop);
					this->GetAvTransportProxy()->BeginStop(instance, functorAsync);
				}				
				
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnStop(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
				// this->GetAvTransportProxy()->EndStop(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::Pause(bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'pause' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{
				if (sync)
					this->GetAvTransportProxy()->SyncPause(instance);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnPause);
					this->GetAvTransportProxy()->BeginPause(instance, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnPause(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndPause(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::Next(bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'next' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetAvTransportProxy()->SyncNext(instance);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnNext);
					this->GetAvTransportProxy()->BeginNext(instance, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnNext(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndNext(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::Previous(bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'previous' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetAvTransportProxy()->SyncPrevious(instance);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnPrevious);
					this->GetAvTransportProxy()->BeginPrevious(instance, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnPrevious(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndPrevious(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::BendAVTransportURI(std::string _uri, std::string _uriMetaData, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'BendAVTransportURI' for renderer '" + this->GetIdentificationString() + "' URI: '" + _uri  + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetAvTransportProxy()->SyncBendAVTransportURI(instance, _uri, _uriMetaData);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnBendAVTransportURI);
					this->GetAvTransportProxy()->BeginBendAVTransportURI(instance, _uri, _uriMetaData, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnBendAVTransportURI(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
			//	this->GetAvTransportProxy()->EndBendAVTransportURI(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::SetAVTransportURI(std::string _uri, std::string _uriMetaData, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'SetAVTransportURI' for renderer '" + this->GetIdentificationString() + "' URI: '" + _uri + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{
				if (sync)
					this->GetAvTransportProxy()->SyncSetAVTransportURI(instance, _uri, _uriMetaData);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSetAVTransportURI);
					this->GetAvTransportProxy()->BeginSetAVTransportURI(instance, _uri, _uriMetaData, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSetAVTransportURI(IAsync& _aAsync)
		{			
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndSetAVTransportURI(_aAsync);
			}
		}
		

		void RaumfeldVirtualMediaRenderer::Seek(boost::int32_t _seekToMs, bool _sync)
		{
			std::string target, unit = "ABS_TIME";
			bool sync = _sync;
			boost::uint32_t	instance = 0;
			char buffer[30];

			time_t timeToSeek(_seekToMs / 1000);
			tm *timeStruct = gmtime(&timeToSeek);			

			if (strftime(buffer, sizeof(buffer), "%H:%M:%S", timeStruct) == 0)
			{
				this->Log(LogType::LOGERROR, "Action 'Seek' for renderer '" + this->GetIdentificationString() + "' Time: '" + target + "' failed!", __FUNCTION__);
				return;
			}
			target = buffer;

			this->Log(LogType::LOGDEBUG, "Action 'Seek' for renderer '" + this->GetIdentificationString() + "' Time: '" + target  + "'", __FUNCTION__);		

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetAvTransportProxy()->SyncSeek(instance, unit, target);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSeek);
					this->GetAvTransportProxy()->BeginSeek(instance, unit, target, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSeek(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndSeek(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::SetPlayMode(MediaRendererPlayMode _playMode, bool _sync)
		{		
			bool sync = _sync;
			boost::uint32_t	instance = 0;		
			std::string playModeString = MediaRenderer::MediaRendererPlayModeToString(_playMode);

			this->Log(LogType::LOGDEBUG, "Action 'SetPlayMode' for renderer '" + this->GetIdentificationString() + "' Mode: '" + playModeString + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{
				if (sync)
					this->GetAvTransportProxy()->SyncSetPlayMode(instance, playModeString);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSetPlayMode);
					this->GetAvTransportProxy()->BeginSetPlayMode(instance, playModeString, functorAsync);
				}
			
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSetPlayMode(IAsync& _aAsync)
		{
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndSetPlayMode(_aAsync);
			}
		}


		AvTransportMediaInfo RaumfeldVirtualMediaRenderer::GetMediaInfo()
		{
			AvTransportMediaInfo mediaInfo;
			bool sync = true;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'GetMediaInfo' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return mediaInfo;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{

				if (sync)
				{
					this->GetAvTransportProxy()->SyncGetMediaInfo(instance, mediaInfo.nrTracks, mediaInfo.mediaDuration, mediaInfo.currentUri, mediaInfo.currentUriMetaData, mediaInfo.nextUri, mediaInfo.nextUriMetaData, mediaInfo.playMedium, mediaInfo.recordMedium, mediaInfo.writeStatus);
					mediaInfo.mediaDurationMS = Utils::TimeStringToTimeMS(mediaInfo.mediaDuration);
				}
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnGetMediaInfo);
					this->GetAvTransportProxy()->BeginGetMediaInfo(instance, functorAsync);
				}
			
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);

			return mediaInfo;
		}

		void RaumfeldVirtualMediaRenderer::OnGetMediaInfo(IAsync& _aAsync)
		{
			if (!this->AvTransportProxyAvailable())
				return;

			// maybee for future use....
			//AvTransportMediaInfo mediaInfo;
			//this->GetAvTransportProxy()->EndGetMediaInfo(_aAsync, mediaInfo.nrTracks, mediaInfo.mediaDuration, mediaInfo.currentUri, mediaInfo.currentUriMetaData, mediaInfo.nextUri, mediaInfo.nextUriMetaData, mediaInfo.playMedium, mediaInfo.recordMedium, mediaInfo.writeStatus);
			// INFO: dont forget to convert durations to integer values!
		}


		AvTransportPositionInfo RaumfeldVirtualMediaRenderer::GetPositionInfo()
		{
			AvTransportPositionInfo positionInfo;
			bool sync = true;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'GetPositionInfo' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->AvTransportProxyAvailable())
				return positionInfo;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{
			
				if (sync)
				{
					this->GetAvTransportProxy()->SyncGetPositionInfo(instance, positionInfo.track, positionInfo.trackDuration, positionInfo.trackMetaData, positionInfo.trackUri, positionInfo.relTime, positionInfo.absTime, positionInfo.relCount, positionInfo.absCount);
					positionInfo.absTimeMS = Utils::TimeStringToTimeMS(positionInfo.absTime);
					positionInfo.relTimeMS = Utils::TimeStringToTimeMS(positionInfo.relTime);
					positionInfo.trackDurationMS = Utils::TimeStringToTimeMS(positionInfo.trackDuration);
				}
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnGetPositionInfo);
					this->GetAvTransportProxy()->BeginGetPositionInfo(instance, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);

			return positionInfo;
		}

		void RaumfeldVirtualMediaRenderer::OnGetPositionInfo(IAsync& _aAsync)
		{
			if (!this->AvTransportProxyAvailable())
				return;

			// maybe for future use....
			//AvTransportPositionInfo positionInfo;
			//this->GetAvTransportProxy()->EndGetPositionInfo(_aAsync, positionInfo.track, positionInfo.trackDuration, positionInfo.trackMetaData, positionInfo.trackUri, positionInfo.relTime, positionInfo.absTime, positionInfo.relCount, positionInfo.absCount);
			// INFO: dont forget to convert durations to integer values!
		}



		void RaumfeldVirtualMediaRenderer::SetMute(bool _mute, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;
			const std::string channel = "Master";

			this->Log(LogType::LOGDEBUG, "Action 'SetMute' for renderer '" + this->GetIdentificationString() + "' Mute: '" + std::to_string(_mute) + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncSetMute(instance, channel, _mute);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSetMute);
					this->GetRenderingControlProxy()->BeginSetMute(instance, channel, _mute, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSetMute(IAsync& _aAsync)
		{
			if (this->RenderingProxyAvailable())
			{
				//this->GetRenderingControlProxy()->EndSetMute(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::SetVolume(boost::uint8_t _volume, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;
			const std::string channel = "Master";

			this->Log(LogType::LOGDEBUG, "Action 'SetVolume' for renderer '" + this->GetIdentificationString() + "' Volume: '" + std::to_string(_volume) + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{							
				if (sync)
					this->GetRenderingControlProxy()->SyncSetVolume(instance, channel, _volume);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSetVolume);
					this->GetRenderingControlProxy()->BeginSetVolume(instance, channel, _volume, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSetVolume(IAsync& _aAsync)
		{
			if (this->RenderingProxyAvailable())
			{				
				//this->GetRenderingControlProxy()->EndSetVolume(_aAsync);
			}
		}



		void RaumfeldVirtualMediaRenderer::SetRoomMute(std::string _roomUDN, bool _mute, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'SetRoomMute' for renderer '" + this->GetIdentificationString() + "' Room: '" + _roomUDN + "' Mute: '" + std::to_string(_mute) + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncSetRoomMute(instance, _roomUDN, _mute);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSetRoomMute);
					this->GetRenderingControlProxy()->BeginSetRoomMute(instance, _roomUDN, _mute, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSetRoomMute(IAsync& _aAsync)
		{
			if (this->RenderingProxyAvailable())
			{
				//this->GetRenderingControlProxy()->EndSetRoomMute(_aAsync);
			}
		}



		void RaumfeldVirtualMediaRenderer::SetRoomVolume(std::string _roomUDN, boost::uint8_t _volume, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'SetRoomVolume' for renderer '" + this->GetIdentificationString() + "' Room: '" + _roomUDN + "' Volume: '" + std::to_string(_volume) + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncSetRoomVolume(instance, _roomUDN, _volume);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnSetRoomVolume);
					this->GetRenderingControlProxy()->BeginSetRoomVolume(instance, _roomUDN, _volume, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnSetRoomVolume(IAsync& _aAsync)
		{
			if (this->RenderingProxyAvailable())
			{
				//this->GetRenderingControlProxy()->EndSetRoomVolume(_aAsync);
			}
		}


		void RaumfeldVirtualMediaRenderer::ChangeVolume(boost::int8_t _amount, bool _sync)
		{
			bool sync = _sync;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'ChangeVolume' for renderer '" + this->GetIdentificationString() + "' Volume: '" + std::to_string(_amount) + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncChangeVolume(instance, _amount);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnChangeVolume);
					this->GetRenderingControlProxy()->BeginChangeVolume(instance, _amount, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);
		}

		void RaumfeldVirtualMediaRenderer::OnChangeVolume(IAsync& _aAsync)
		{
			if (this->RenderingProxyAvailable())
			{
				//this->GetRenderingControlProxy()->EndChangeVolume(_aAsync);
			}
		}


		bool RaumfeldVirtualMediaRenderer::GetMute()
		{
			bool sync = true, mute;
			boost::uint32_t	instance = 0;
			const std::string channel = "Master";

			this->Log(LogType::LOGDEBUG, "Action 'GetMute' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return false;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			

				if (sync)
					this->GetRenderingControlProxy()->SyncGetMute(instance, channel, mute);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnGetMute);
					this->GetRenderingControlProxy()->BeginGetMute(instance, channel, functorAsync);
				}

			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);

			return mute;
		}

		void RaumfeldVirtualMediaRenderer::OnGetMute(IAsync& _aAsync)
		{
			if (!this->RenderingProxyAvailable())
				return;

			// maybe for future use....
			//bool mute;
			//this->GetRenderingControlProxy()->EndGetMute(_aAsync, mute);
		}


		boost::uint8_t RaumfeldVirtualMediaRenderer::GetVolume()
		{
			bool sync = true;
			boost::uint32_t volume;
			boost::uint32_t	instance = 0;
			const std::string channel = "Master";

			this->Log(LogType::LOGDEBUG, "Action 'GetVolume' for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return 0;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncGetVolume(instance, channel, volume);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnGetVolume);
					this->GetRenderingControlProxy()->BeginGetVolume(instance, channel, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);

			return volume;
		}

		void RaumfeldVirtualMediaRenderer::OnGetVolume(IAsync& _aAsync)
		{
			if (!this->RenderingProxyAvailable())
				return;

			// maybe for future use....
			//boost::uint32_t volume;
			//this->GetRenderingControlProxy()->EndGetVolume(_aAsync, volume);
		}


		bool RaumfeldVirtualMediaRenderer::GetRoomMute(std::string _roomUDN)
		{
			bool sync = true, mute;
			boost::uint32_t	instance = 0;		

			this->Log(LogType::LOGDEBUG, "Action 'GetRoomMute' for renderer '" + this->GetIdentificationString() + "' Room: '" + _roomUDN + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return false;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncGetRoomMute(instance, _roomUDN, mute);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnGetRoomMute);
					this->GetRenderingControlProxy()->BeginGetRoomMute(instance, _roomUDN, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);

			return mute;
		}

		void RaumfeldVirtualMediaRenderer::OnGetRoomMute(IAsync& _aAsync)
		{
			if (!this->RenderingProxyAvailable())
				return;

			// maybe for future use....
			//bool mute;
			//this->GetRenderingControlProxy()->EndGetRoomMute(_aAsync, mute);
		}



		boost::uint8_t RaumfeldVirtualMediaRenderer::GetRoomVolume(std::string _roomUDN)
		{
			bool sync = true;
			boost::uint32_t volume;
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "Action 'GetRoomVolume' for renderer '" + this->GetIdentificationString() + "' Room: '" + _roomUDN + "'", __FUNCTION__);

			if (!this->RenderingProxyAvailable())
				return 0;

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{			
				if (sync)
					this->GetRenderingControlProxy()->SyncGetRoomVolume(instance, _roomUDN, volume);
				else
				{
					OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnGetRoomVolume);
					this->GetRenderingControlProxy()->BeginGetRoomVolume(instance, _roomUDN, functorAsync);
				}
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(false);

			return volume;
		}

		void RaumfeldVirtualMediaRenderer::OnGetRoomVolume(IAsync& _aAsync)
		{
			if (!this->RenderingProxyAvailable())
				return;

			// maybe for future use....
			//boost::uint32_t  volume;
			//this->GetRenderingControlProxy()->EndGetRoomVolume(_aAsync, volume);
		}


		void RaumfeldVirtualMediaRenderer::AddSubscriptions()
		{					
			try
			{

				this->Log(LogType::LOGDEBUG, "Adding subscriptions for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

				OpenHome::Functor functor = OpenHome::MakeFunctor(*this, &RaumfeldVirtualMediaRenderer::OnRenderingServicePropertyChanged);
				OpenHome::Functor functorAV = OpenHome::MakeFunctor(*this, &RaumfeldVirtualMediaRenderer::OnAVTransportServicePropertyChanged);
				OpenHome::Functor functorCM = OpenHome::MakeFunctor(*this, &RaumfeldVirtualMediaRenderer::OnConnectionManagerPropertyChanged);
				OpenHome::Functor functorCMConIds = OpenHome::MakeFunctor(*this, &RaumfeldVirtualMediaRenderer::OnConnectionManagerConnectionIdsChanged);

				this->GetRenderingControlProxy()->SetPropertyChanged(functor);
				this->GetRenderingControlProxy()->SetPropertyInitialEvent(functor);
				this->GetRenderingControlProxy()->SetPropertyLastChangeChanged(functor);
				this->GetRenderingControlProxy()->Subscribe();

				this->GetAvTransportProxy()->SetPropertyChanged(functorAV);
				this->GetAvTransportProxy()->SetPropertyInitialEvent(functorAV);
				this->GetAvTransportProxy()->SetPropertyLastChangeChanged(functorAV);
				this->GetAvTransportProxy()->Subscribe();

				this->GetConnectionManagerProxy()->SetPropertyChanged(functorCM);
				this->GetConnectionManagerProxy()->SetPropertyInitialEvent(functorCM);
				this->GetConnectionManagerProxy()->SetPropertyCurrentConnectionIDsChanged(functorCMConIds);
				this->GetConnectionManagerProxy()->Subscribe();			
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
		}

		void RaumfeldVirtualMediaRenderer::RemoveSubscriptions()
		{

			try
			{
				// use this method to add subscriptions to upnp services
				this->Log(LogType::LOGDEBUG, "Removing subscriptions for renderer '" + this->GetIdentificationString() + "'", __FUNCTION__);

				if (avTransportProxy) avTransportProxy->Unsubscribe();
				if (renderingControlProxy) renderingControlProxy->Unsubscribe();
				if (connectionManagerProxy) connectionManagerProxy->Unsubscribe();
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}

		}


		MediaRendererRoomState RaumfeldVirtualMediaRenderer::GetMediaRendererRoomState(std::string _roomUDN)
		{		
			MediaRendererRoomState defaultValue;
			boost::unordered_map<std::string, MediaRendererRoomState>::iterator it = roomStateMap.find(Utils::FormatUDN(_roomUDN));
			if (it != roomStateMap.end())
				return it->second;
			return defaultValue;		
		}


		void RaumfeldVirtualMediaRenderer::OnConnectionManagerConnectionIdsChanged()
		{
			std::string propertyXML = "";

			this->Log(LogType::LOGDEBUG, "ConnectionManager on '" + this->GetIdentificationString() + "': Connection Id's changed!", __FUNCTION__);

			this->GetConnectionManagerProxy()->PropertyCurrentConnectionIDs(propertyXML);
		}


		void RaumfeldVirtualMediaRenderer::OnConnectionManagerPropertyChanged()
		{
			std::string propertyXML = "";

			this->Log(LogType::LOGDEBUG, "ConnectionManager on '" + this->GetIdentificationString() + "': property changed!", __FUNCTION__);

			// no properties????
		}

		void RaumfeldVirtualMediaRenderer::AddPropChangeInfo(std::string _propertyName, std::string &_propChangeInfo)
		{
			if (!_propChangeInfo.empty())
				_propChangeInfo += ", ";
			_propChangeInfo += _propertyName;
		};


		void RaumfeldVirtualMediaRenderer::OnAVTransportServicePropertyChanged()
		{
			std::string propertyXML = "", attributeString = "", curentTrackMetadata = "", contentType = "", aVTransportURIMetaData = "", aVTransportURI = "", currentTrackURI = "", currentTrackDuration = "", propChangeInfo = "";
			std::string uriQuery;
			boost::uint32_t currentTrackDurationMS = 0, currentTrack = 0, numberOfTracks = 0, bitrate = 0;
			MediaRendererTransportState transportState;
			xml_document<> doc;
			xml_node<> *eventNode, *instanceNode, *valueNode;
			xml_attribute<> *attribute;			
			bool someStateChanged = false;	
			bool avTransportUriChanged = false;
		

			this->Log(LogType::LOGDEBUG, "AVTransportService on '" + this->GetIdentificationString() + "': property changed!", __FUNCTION__);

			this->GetAvTransportProxy()->PropertyLastChange(propertyXML);

			char* cstr = new char[propertyXML.size() + 1];
			strcpy(cstr, propertyXML.c_str());
			doc.parse<0>(cstr);

			eventNode = doc.first_node("Event", 0, false);
			if (!eventNode)
			{
				this->Log(LogType::LOGERROR, "RenderingService on '" + this->GetIdentificationString() + "': 'Event' node in propertyXML not found!", __FUNCTION__);
				return;
			}

			instanceNode = eventNode->first_node("InstanceId", 0, false);
			if (!instanceNode)
			{
				this->Log(LogType::LOGERROR, "RenderingService on '" + this->GetIdentificationString() + "': 'InstanceId' node in propertyXML not found!", __FUNCTION__);
				return;
			}

			try
			{
								
 				boost::mutex::scoped_lock scoped_lock(mutexRendererState);			

				valueNode = instanceNode->first_node("CurrentTrackMetaData", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					curentTrackMetadata = attribute->value();
					if (mediaRendererState.currentTrackMetaData != curentTrackMetadata)
					{
						mediaRendererState.currentTrackMetaData = curentTrackMetadata;
						someStateChanged = true;
						AddPropChangeInfo("CurrentTrackMetaData", propChangeInfo);

						// parse into mediaItem track class					
						currentMediaItem = managerList.contentManager->CreateMediaItemFromCurrentTrackMetadata(curentTrackMetadata);
					}
				}

				valueNode = instanceNode->first_node("ContentType", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					contentType = attribute->value();
					if (mediaRendererState.contentType != contentType)
					{
						mediaRendererState.contentType = contentType;
						someStateChanged = true;
						AddPropChangeInfo("ContentType", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("AVTransportURI", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					aVTransportURI = attribute->value();
					if (mediaRendererState.aVTransportURI != aVTransportURI)
					{
						mediaRendererState.aVTransportURI = aVTransportURI;

						// in some cases it may be that the avTransportUri es empty

						if (!aVTransportURI.empty())
						{
							// we have to handle the query values of the AVTransportUri.
							// In case we wan't to play eg. Track 5 of a list, we have to use the cid(containerId) of the container the renderer has set currently
							// so this is the place where the actual cid is set to the 'mediaRendererState' structure
							boost::network::uri::uri instance(aVTransportURI);
							uriQuery = instance.query();

							// parse the query string for all id and value pairs it may have
							boost::unordered_map<std::string, std::string> queryValues = Utils::ParseQueryString(uriQuery);

							// now handle the cid(containerId) information. if its there, then get it and store it
							boost::unordered_map<std::string, std::string>::iterator it = queryValues.find("cid");
							if (it != queryValues.end())
							{
								std::string containerIdEncoded = it->second;
								// we want to store the unescaped value due the cid will be escaped again on bending AVTransport Uri
								// UPDATE: no need to Unescape the values! This is already done by 'ParseQueryString'
								//mediaRendererState.containerId = Utils::Unescape(containerIdEncoded);
								mediaRendererState.containerId = containerIdEncoded;
							}
							else
								mediaRendererState.containerId = "";

							// TODO: handle fid && fii		

							someStateChanged = true;
							avTransportUriChanged = true;
							AddPropChangeInfo("AVTransportURI", propChangeInfo);

						}
					}
				}

				valueNode = instanceNode->first_node("AVTransportURIMetaData", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					aVTransportURIMetaData = attribute->value();
					if (mediaRendererState.aVTransportURIMetaData != aVTransportURIMetaData)
					{
						mediaRendererState.aVTransportURIMetaData = aVTransportURIMetaData;
						someStateChanged = true;
						AddPropChangeInfo("AVTransportURIMetaData", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("CurrentTrackURI", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					currentTrackURI = attribute->value();
					if (mediaRendererState.currentTrackURI != currentTrackURI)
					{
						mediaRendererState.currentTrackURI = currentTrackURI;
						someStateChanged = true;
						AddPropChangeInfo("CurrentTrackURI", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("CurrentTrackDuration", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					currentTrackDuration = attribute->value();
					currentTrackDurationMS = Utils::TimeStringToTimeMS(currentTrackDuration);
					if (mediaRendererState.currentTrackDuration != currentTrackDuration)
					{
						mediaRendererState.currentTrackDuration = currentTrackDuration;
						mediaRendererState.currentTrackDurationMS = currentTrackDurationMS;
						someStateChanged = true;
						AddPropChangeInfo("CurrentTrackDuration", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("CurrentTrack", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					attributeString = attribute->value();
					currentTrack = this->toInt(attributeString);
					if (mediaRendererState.currentTrack != currentTrack)
					{
						mediaRendererState.currentTrack = currentTrack;
						someStateChanged = true;
						AddPropChangeInfo("CurrentTrack", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("NumberOfTracks", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					attributeString = attribute->value();
					numberOfTracks = this->toInt(attributeString);
					if (mediaRendererState.numberOfTracks != numberOfTracks)
					{
						mediaRendererState.numberOfTracks = numberOfTracks;
						someStateChanged = true;
						AddPropChangeInfo("NumberOfTracks", propChangeInfo);
					}

				}

				valueNode = instanceNode->first_node("Bitrate", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					attributeString = attribute->value();
					bitrate = this->toInt(attributeString);
					if (mediaRendererState.bitrate != bitrate)
					{
						mediaRendererState.bitrate = bitrate;
						someStateChanged = true;
						AddPropChangeInfo("Bitrate", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("TransportState", 0, false);
				if (valueNode)
				{
					attribute = valueNode->first_attribute("val");
					attributeString = attribute->value();
					transportState = StringToMediaRendererTransportState(attributeString);
					if (mediaRendererState.transportState != transportState)
					{
						mediaRendererState.transportState = transportState;
						someStateChanged = true;
						AddPropChangeInfo("TransportState", propChangeInfo);
					}
				}

				valueNode = instanceNode->first_node("RoomStates", 0, false);
				if (valueNode)
				{
					std::vector<std::string> roomVolumeStringVector;
					std::vector<std::string> roomStringVector;
					std::string roomUDN;
					MediaRendererTransportState	roomTransportState;
					MediaRendererRoomState roomState;

					attribute = valueNode->first_attribute("val");
					attributeString = attribute->value();
				
					if (!attributeString.empty())
					{

						roomVolumeStringVector = Utils::ExplodeString(attributeString, ",");

						for (auto &roomString : roomVolumeStringVector)
						{
							roomStringVector = Utils::ExplodeString(roomString, "=");
							if (roomStringVector.size() > 1)
							{
								roomUDN = Utils::FormatUDN(roomStringVector[0]);
								roomTransportState = StringToMediaRendererTransportState(roomStringVector[1]);

								if (roomStateMap.find(roomUDN) != roomStateMap.end())
								{
									roomState = roomStateMap.at(roomUDN);
									roomStateMap.erase(roomUDN);
								}

								roomState.roomUDN = roomUDN;
								if (roomState.transportState != roomTransportState)
								{
									roomState.transportState = roomTransportState;
									someStateChanged = true;
									AddPropChangeInfo("RoomStates", propChangeInfo);
								}

								roomStateMap.insert(std::make_pair(roomUDN, roomState));
							}
						}
					}

				}		
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
			}

			// if soemthing has changed we do signal the subscribers
			if (someStateChanged)
			{
				// each update of the zones we generate and store a random number
				// this is needed for JSON webserver for long polling action on zone information
				boost::uint32_t curUpdateId = Utils::GetRandomNumber();
				if (managerList.deviceManager->GetLastUpdateIdRendererState() == curUpdateId)
					curUpdateId++;
				managerList.deviceManager->SetLastUpdateIdRendererState(curUpdateId);

				// start loading the playlist for the renderer. Virtual Renderer UDN = virtual Zone UDN
				if (avTransportUriChanged)
					managerList.contentManager->StartGetMediaItemListByZoneUDN(this->GetUDN(), false);				

				this->Log(LogType::LOGDEBUG, "Properties Changed: " + propChangeInfo + " (" + this->GetIdentificationString() + ")", __FUNCTION__);
				signalMediaRendererStateChanged(this->UDN);
			}
		}


		void RaumfeldVirtualMediaRenderer::OnRenderingServicePropertyChanged()
		{
			// Example XML

			/* 
			<Event xmlns="urn:schemas-upnp-org:metadata-1-0/RCS/">
			<InstanceID val="0">
			<RoomVolumes val="uuid:e1104a23-2493-498c-8c44-49ba8d1810d8=44,uuid:6e395fd9-f326-41c6-b8b3-eed1897923e6=0" />
			<Volume channel="Master" val="22" />
			<RoomMutes val="uuid:e1104a23-2493-498c-8c44-49ba8d1810d8=0,uuid:6e395fd9-f326-41c6-b8b3-eed1897923e6=0" />
			</InstanceID>
			</Event>
			*/
			
			std::string propertyXML = "", attributeString = "", propChangeInfo = "";
			xml_document<> doc;
			xml_node<> *eventNode, *instanceNode, *roomVolumesNode, *volumeNode, *muteNode, *roomMutesNode;
			xml_attribute<> *attribute;
			boost::uint8_t masterVolume = mediaRendererState.volume, masterMute = 0;
			MediaRendererMuteState masterMuteState = mediaRendererState.muteState;
			bool minOneRoomMuted = false, allRoomsMuted = true, someStateChanged = false;;
			boost::unordered_map<std::string, MediaRendererRoomState>	roomStateMapLocal;				

			this->Log(LogType::LOGDEBUG, "RenderingService on '" + this->GetIdentificationString() + "': property changed!", __FUNCTION__);
				
			this->GetRenderingControlProxy()->PropertyLastChange(propertyXML);

			// this xml vontains the proprty values for the rendering ervice which normaly are 'volume level' and 'mute state'
			// in this case for rooms and the zone (master)

			if (propertyXML.empty())
			{
				this->Log(LogType::LOGWARNING, "RenderingService on '" + this->GetIdentificationString() + "': property changed has no XML!", __FUNCTION__);
				return;
			}

			// add lock because ware are going on to update our lists						
			boost::mutex::scoped_lock scoped_lock(mutexRendererState);
			
		
			char* cstr = new char[propertyXML.size() + 1];
			strcpy(cstr, propertyXML.c_str());
			doc.parse<0>(cstr);						
			
			eventNode = doc.first_node("Event", 0, false);
			if (!eventNode)
			{
				this->Log(LogType::LOGERROR, "RenderingService on '" + this->GetIdentificationString() + "': 'Event' node in propertyXML not found!", __FUNCTION__);
				return;
			}

			instanceNode = eventNode->first_node("InstanceId", 0, false);
			if (!instanceNode)
			{
				this->Log(LogType::LOGERROR, "RenderingService on '" + this->GetIdentificationString() + "': 'InstanceId' node in propertyXML not found!", __FUNCTION__);
				return;
			}

			//this->GetDeviceManager()->LockRendererServerListForUpdate(true);

			try
			{
				
				volumeNode = instanceNode->first_node("Volume", 0, false);
				if (volumeNode)
				{				
					attribute = volumeNode->first_attribute("val");				
					attributeString = attribute->value();
					masterVolume = this->toInt(attributeString);
				}

				roomVolumesNode = instanceNode->first_node("RoomVolumes", 0, false);
				if (roomVolumesNode)
				{		
					std::vector<std::string> roomVolumeStringVector;
					std::vector<std::string> roomStringVector;
					std::string roomUDN, roomVolume;
					MediaRendererRoomState roomState;

					// volume node value looks like this "uuid:e1104a23-2493-498c-8c44-49ba8d1810d8=44,uuid:6e395fd9-f326-41c6-b8b3-eed1897923e6=0"
					// it consists of the room uid and the volume value
					attribute = roomVolumesNode->first_attribute("val");
					attributeString = attribute->value();

					roomVolumeStringVector = Utils::ExplodeString(attributeString, ",");
				
					for (auto &roomString : roomVolumeStringVector)
					{
						roomStringVector = Utils::ExplodeString(roomString, "=");
						if (roomStringVector.size() >= 2)
						{
							roomUDN = Utils::FormatUDN(roomStringVector[0]);
							roomVolume = roomStringVector[1];

							if (roomStateMapLocal.find(roomUDN) != roomStateMapLocal.end())
							{
								roomState = roomStateMapLocal.at(roomUDN);
								roomStateMapLocal.erase(roomUDN);
							}

							roomState.roomUDN = roomUDN;
							roomState.volume = this->toInt(roomVolume);

							roomStateMapLocal.insert(std::make_pair(roomUDN, roomState));
						}
					}

				}

				roomMutesNode = instanceNode->first_node("RoomMutes", 0, false);
				if (roomMutesNode)
				{
					std::vector<std::string> roomMuteStringVector;
					std::vector<std::string> roomStringVector;
					std::string roomUDN, roomMute;
					MediaRendererRoomState roomState;

					// mute node value looks like this "uuid:e1104a23-2493-498c-8c44-49ba8d1810d8=1,uuid:6e395fd9-f326-41c6-b8b3-eed1897923e6=0"
					// it consists of the room uid and the volume value
					attribute = roomMutesNode->first_attribute("val");
					attributeString = attribute->value();

					roomMuteStringVector = Utils::ExplodeString(attributeString, ",");

					for (auto &roomString : roomMuteStringVector)
					{
						roomStringVector = Utils::ExplodeString(roomString, "=");
						if (roomStringVector.size() >= 2)
						{

							roomUDN = Utils::FormatUDN(roomStringVector[0]);
							roomMute = roomStringVector[1];

							if (roomStateMapLocal.find(roomUDN) != roomStateMapLocal.end())
							{
								roomState = roomStateMapLocal.at(roomUDN);
								roomStateMapLocal.erase(roomUDN);
							}

							roomState.roomUDN = roomUDN;
							roomState.mute = this->toInt(roomMute) != 0;

							if (roomState.mute)
								minOneRoomMuted = true;
							else
								allRoomsMuted = false;

							roomStateMapLocal.insert(std::make_pair(roomUDN, roomState));
						}
					}	

					if (allRoomsMuted && minOneRoomMuted)
						masterMuteState = MediaRendererMuteState::MRPMUTE_ALL;
					else if (minOneRoomMuted)
						masterMuteState = MediaRendererMuteState::MRPMUTE_PARTIAL;
					else
						masterMuteState = MediaRendererMuteState::MRPMUTE_NONE;

				}

				muteNode = instanceNode->first_node("Mute", 0, false);
				if (muteNode)
				{
					attribute = muteNode->first_attribute("val");
					attributeString = attribute->value();
					masterMute = this->toInt(attributeString);
					if (masterMute == 1)
						masterMuteState = MediaRendererMuteState::MRPMUTE_ALL;
					else if (minOneRoomMuted)
						masterMuteState = MediaRendererMuteState::MRPMUTE_PARTIAL;
					else
						masterMuteState = MediaRendererMuteState::MRPMUTE_NONE;
				}

			

		
				someStateChanged = false;

				// check change of value and signal those ones (to deviceManager, and this one signals out to whatever has attached to the signal)
				if (mediaRendererState.volume != masterVolume)			
				{
					mediaRendererState.volume = masterVolume;
					someStateChanged = true;
					AddPropChangeInfo("Volume", propChangeInfo);
				}
				if (mediaRendererState.muteState != masterMuteState)
				{
					mediaRendererState.muteState = masterMuteState;
					someStateChanged = true;
					AddPropChangeInfo("Mute", propChangeInfo);
				}

				// update room states
				MediaRendererRoomState roomStateLocal, roomState;

				for (auto &mapItem : roomStateMapLocal)
				{
					roomStateLocal = (MediaRendererRoomState)mapItem.second;

					if (roomStateMap.find(roomStateLocal.roomUDN) != roomStateMap.end())
					{
						roomState = roomStateMap.at(roomStateLocal.roomUDN);
						roomStateMap.erase(roomStateLocal.roomUDN);
					}

					if (roomState.roomUDN != roomStateLocal.roomUDN)
					{
						roomState.roomUDN = roomStateLocal.roomUDN;
						someStateChanged = true;
						AddPropChangeInfo("RoomUDN", propChangeInfo);
					}
					if (roomState.volume != roomStateLocal.volume)
					{
						roomState.volume = roomStateLocal.volume;
						someStateChanged = true;
						AddPropChangeInfo("RoomState", propChangeInfo);
					}
					if (roomState.mute != roomStateLocal.mute)
					{
						roomState.mute = roomStateLocal.mute;
						someStateChanged = true;
						AddPropChangeInfo("RoomMute", propChangeInfo);
					}
				
					roomStateMap.insert(std::make_pair(roomState.roomUDN, roomState));
				}						

			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}			

			// if soemthing has changed we do signal the subscribers
			if (someStateChanged)
			{
				// each update of the zones we generate and store a random number
				// this is needed for JSON webserver for long polling action on zone information
				boost::uint32_t curUpdateId = Utils::GetRandomNumber();
				if (managerList.deviceManager->GetLastUpdateIdRendererState() == curUpdateId)
					curUpdateId++;
				managerList.deviceManager->SetLastUpdateIdRendererState(curUpdateId);

				this->Log(LogType::LOGDEBUG, "Properties Changed: " + propChangeInfo + " (" + this->GetIdentificationString() + ")", __FUNCTION__);
				signalMediaRendererStateChanged(this->UDN);
			}

		}

		
		RaumfeldMediaServer* RaumfeldVirtualMediaRenderer::GetMediaServer()
		{
			
			DeviceManager *deviceManager = this->GetDeviceManager();
			if (deviceManager ==nullptr)
				return nullptr;

			return (RaumfeldMediaServer*)deviceManager->GetMediaServer();
			
		}


		// RaumfeldVirtualMediaRenderer::PlayTrack		
		void RaumfeldVirtualMediaRenderer::PlayTrack(boost::int32_t _trackIndex, bool _sync)
		{
			this->Log(LogType::LOGDEBUG, "PlayTrack: Track numer:" + std::to_string(_trackIndex) + " (" + this->GetIdentificationString() + ")", __FUNCTION__);

			if (_trackIndex < 0)
			{
				this->Play(_sync);
			}
			else
			{
				//this->Stop(true);
				//this->BindContainer(mediaRendererState.containerId, _trackIndex, true);		
				this->SetContainer(mediaRendererState.containerId, _trackIndex, true);						
				this->Play(_sync);
			};
		}


		// RaumfeldVirtualMediaRenderer::LoadUri
		void RaumfeldVirtualMediaRenderer::LoadUri(std::string _uri, bool _play)
		{			
			std::string AVTRansportUri;

			this->Log(LogType::LOGDEBUG, "LoadUri: " + _uri + " (" + this->GetIdentificationString() + ")", __FUNCTION__);	

			if (this->GetAvTransportProxy() == nullptr)
			{
				this->Log(LogType::LOGWARNING, "LoadUri failed: No AVTransportProxy present! (" + this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			this->Stop(true);
			// TODO: Where to get the metadata?
			this->SetAVTransportURI(_uri, "", true);
			if (_play)
				this->Play(true);
			//TODO: i dont know why this stop is needed. Sometimes it starts playing without any reason. I have to test this	
			else
				this->Stop(true);
		}

		// RaumfeldVirtualMediaRenderer::LoadSingle
		void RaumfeldVirtualMediaRenderer::LoadSingle(std::string _singleId, bool _play)
		{
			std::string	AVTRansportUri;
			RaumfeldMediaServer *mediaServer = this->GetMediaServer();
			boost::uint32_t	instance = 0;

			this->Log(LogType::LOGDEBUG, "LoadSingle: " + _singleId + " (" + this->GetIdentificationString() + ")", __FUNCTION__);

			if (mediaServer == nullptr)
			{
				this->Log(LogType::LOGWARNING, "LoadSingle failed: No MediaServer present! (" + this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			if (this->GetAvTransportProxy() == nullptr)
			{
				this->Log(LogType::LOGWARNING, "LoadSingle failed: No AVTransportProxy present! (" + this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			this->Stop(true);

			AVTRansportUri = mediaServer->CreateAVTransportUri_Single(_singleId);
			// TODO: Where to get the metadata?
			this->GetAvTransportProxy()->SyncSetAVTransportURI(instance, AVTRansportUri, "");		
			if (_play)
				this->Play(true);
			//TODO: i dont know why this stop is needed. Sometimes it starts playing without any reason. I have to test this	
			else
				this->Stop(true);
		}


		// RaumfeldVirtualMediaRenderer::LoadContainer
		// Attention: "_containerId" has to be correct formated!
		void RaumfeldVirtualMediaRenderer::LoadContainer(std::string _containerId, boost::int32_t _trackIndex, bool _play)
		{
			if (_trackIndex < 0)
				_trackIndex = 0;

			this->Log(LogType::LOGDEBUG, "LoadContainer: " + _containerId + " (" + this->GetIdentificationString() + ")", __FUNCTION__);
			
			this->Stop(true);			
			this->BindContainer(_containerId, _trackIndex, true);
			if (_play)
				this->Play(true);
			//TODO: i dont know why this stop is needed. Sometimes it starts playing without any reason. I have to test this	
			else
				this->Stop(true);
		}	


		// RaumfeldVirtualMediaRenderer::BindContainer
		// Attention: "_containerId" has to be correct formated!
		void RaumfeldVirtualMediaRenderer::BindContainer(std::string _containerId, boost::int32_t _trackIndex, bool _sync)
		{
			bool sync = _sync;
			std::string	AVTRansportUri;
			RaumfeldMediaServer *mediaServer = this->GetMediaServer();
			boost::uint32_t	instance = 0;
			OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnBendAvTransportUri);


			if (mediaServer == nullptr)
			{
				this->Log(LogType::LOGWARNING, "BindContainer failed: No MediaServer present! (" + this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			if (this->GetAvTransportProxy() == nullptr)
			{
				this->Log(LogType::LOGWARNING, "BindContainer failed: No AVTransportProxy present! (" +  this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			this->Log(LogType::LOGDEBUG, "BindContainer: " + _containerId +  " (" + this->GetIdentificationString() + ")", __FUNCTION__);

			if (Utils::TrackIndexToTrackNumber(_trackIndex) > mediaRendererState.numberOfTracks)
				_trackIndex = Utils::TrackNumberToTrackIndex(mediaRendererState.numberOfTracks);
					
			AVTRansportUri = mediaServer->CreateAVTransportUri_Container(_containerId, _trackIndex);

			// a valid transport uri looks like this!
			//dlna-playcontainer://uuid%3Aed3bd3db-17b1-4dbe-82df-5201c78e632c?sid=urn%3Aupnp-org%3AserviceId%3AContentDirectory&cid=0%2FPlaylists%2FMyPlaylists%2FTest&md=0						

			// TODO: Metadata comes from CreateQueue???
			try
			{
				if (sync)
					this->GetAvTransportProxy()->SyncBendAVTransportURI(instance, AVTRansportUri, "");
				else
					this->GetAvTransportProxy()->BeginBendAVTransportURI(instance, AVTRansportUri, "", functorAsync);
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
		}

		// RaumfeldVirtualMediaRenderer::SetContainer
		// Attention: "_containerId" has to be correct formated!
		void RaumfeldVirtualMediaRenderer::SetContainer(std::string _containerId, boost::int32_t _trackIndex, bool _sync)
		{
			bool sync = _sync;
			std::string	AVTRansportUri;
			RaumfeldMediaServer *mediaServer = this->GetMediaServer();
			boost::uint32_t	instance = 0;
			OpenHome::Net::FunctorAsync functorAsync = OpenHome::Net::MakeFunctorAsync(*this, &RaumfeldVirtualMediaRenderer::OnBendAvTransportUri);


			if (mediaServer == nullptr)
			{
				this->Log(LogType::LOGWARNING, "SetContainer failed: No MediaServer present! (" + this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			if (this->GetAvTransportProxy() == nullptr)
			{
				this->Log(LogType::LOGWARNING, "SetContainer failed: No AVTransportProxy present! (" + this->GetIdentificationString() + ")", __FUNCTION__);
				return;
			}

			this->Log(LogType::LOGDEBUG, "SetContainer: " + _containerId + " (" + this->GetIdentificationString() + ")", __FUNCTION__);

			if (Utils::TrackIndexToTrackNumber(_trackIndex) > mediaRendererState.numberOfTracks)
				_trackIndex = Utils::TrackNumberToTrackIndex(mediaRendererState.numberOfTracks);

			AVTRansportUri = mediaServer->CreateAVTransportUri_Container(_containerId, _trackIndex);

			// a valid transport uri looks like this!
			//dlna-playcontainer://uuid%3Aed3bd3db-17b1-4dbe-82df-5201c78e632c?sid=urn%3Aupnp-org%3AserviceId%3AContentDirectory&cid=0%2FPlaylists%2FMyPlaylists%2FTest&md=0						

			// TODO: Metadata comes from CreateQueue???
			try
			{
				if (sync)
					this->GetAvTransportProxy()->SyncSetAVTransportURI(instance, AVTRansportUri, "");
				else
					this->GetAvTransportProxy()->BeginSetAVTransportURI(instance, AVTRansportUri, "", functorAsync);
			}
			catch (OpenHome::Exception &ex)
			{
				this->UPNPException(ex, __FUNCTION__);
			}
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
		}
	

		void RaumfeldVirtualMediaRenderer::OnBendAvTransportUri(IAsync& _aAsync)
		{	
			if (this->AvTransportProxyAvailable())
			{
				//this->GetAvTransportProxy()->EndBendAVTransportURI(_aAsync);
			}
		}

		MediaItem RaumfeldVirtualMediaRenderer::GetCurrentMediaItem()
		{
			return currentMediaItem;
		}

		
		/*

		// FOLLOWING METHODS TO MEDIA SERVER!!!

		// creates a queue from current list if it is not a queue
		protected virtual Boolean createQueue()
		{
		System.String givenName, queueIdCreated;
		uint cuid;

		if (this.isQueue())
		return false;

		contentDirectory.CreateQueueSync(listId, "0/Zones", out givenName, out queueIdCreated, out containerInfoMetaData);

		if(!String.IsNullOrWhiteSpace(queueIdCreated))
		this.writeLog(LogType.Error, String.Format("Fehler beim erstellen einer queue aus containerID '{0}'", containerId));

		contentDirectory.RemoveFromQueueSync(queueIdCreated, 0, 4294967295, out cuid);

		if (this.list.Count > 0)
		{
		// move container or item to queue (Sync)
		if (!String.IsNullOrEmpty(containerId))
		contentDirectory.AddContainerToQueueSync(queueIdCreated, containerId, containerId, "", "", 0, 4294967295, 0);
				else
					// there hast to be only one object in list! otherwise it would be a container ora a queue!
					contentDirectory.AddItemToQueueSync(queueIdCreated, this.list[0].objectId, 0);
			}

			containerId = queueIdCreated;

			// reread list from new queue! (Sync!) we have to get the new list with the new ids!!!
			this.retrieveListByContainerId(containerId, "*", true);

			return true;
		}
		
		*/

		/*
		
		// returns true if current loaded list is a queue or not
		// a queue is defined by the 'name' for now. That means if there is some special text in the containerId, then its a queue
		public virtual Boolean isQueue()
		{
		if (!String.IsNullOrEmpty(containerId))
		return containerId.IndexOf("0/Zones/") != -1;
		return false;
		}

		*/
	
		

	
}