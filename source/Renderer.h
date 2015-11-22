#ifndef RENDERER_H
#define RENDERER_H


#include "os.h"

#include <string>
#include <fstream>
#include <vector>
#include <time.h>

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/unordered_map.hpp>

#include <OpenHome/Net/Cpp/OhNet.h>
#include <OpenHome/Net/Cpp/CpDevice.h>
#include <OpenHome/Net/Cpp/CpDeviceDv.h>
#include <OpenHome/Net/Cpp/CpDeviceUpnp.h>
#include <OpenHome/Net/Core/CpProxy.h>

#include "Logger.h"
#include "Global.h"
#include "RaumkernObject.h"
#include "MediaItem.h"

// we do have 3 proxies vor the AVTransport service. one for standard renderers, one for raumfeld renderers and on for the virtual raumfeld rendereres
// due we only do actions on virtual renderers (zones) we do not need the other 2 one for now!
#include  "proxies/CpUpnpOrgAVTransport_RaumfeldVirtual1.h"
#include  "proxies/CpUpnpOrgConnectionManager_RaumfeldVirtual1.h"
#include  "proxies/CpUpnpOrgRenderingControl_RaumfeldVirtual1.h"


using namespace OpenHome::Net;


namespace Raumkernel
{		
		class DeviceManager;
		class MediaServer;
		class RaumfeldMediaServer;

		enum class MediaRendererPlayMode { MRPLAYMODE_NORMAL = 0, MRPLAYMODE_SHUFFLE = 1, MRPLAYMODE_REPEAT_ONE = 2, MRPLAYMODE_REPEAT_ALL = 3, MRPLAYMODE_RANDOM = 4, MRPLAYMODE_DIRECT_1 = 5, MRPLAYMODE_INTRO = 6 };

		enum class MediaRendererMuteState { MRPMUTE_NONE, MRPMUTE_ALL, MRPMUTE_PARTIAL };

		enum class MediaRendererTransportState { MRTS_STOPPED, MRTS_PLAYING, MRTS_TRANSITIONING};
	

		struct AvTransportMediaInfo
		{
			boost::uint32_t nrTracks;
			std::string mediaDuration;
			boost::uint32_t mediaDurationMS;
			std::string currentUri;
			std::string currentUriMetaData;
			std::string nextUri;
			std::string nextUriMetaData;
			std::string playMedium;
			std::string recordMedium;
			std::string writeStatus;
		};

		struct AvTransportPositionInfo
		{
			boost::uint32_t track;
			std::string trackDuration;
			boost::uint32_t trackDurationMS;
			std::string trackMetaData;
			std::string trackUri;
			std::string relTime;
			boost::uint32_t relTimeMS;
			std::string absTime;
			boost::uint32_t absTimeMS;
			boost::int32_t relCount;
			boost::int32_t absCount;
		};


		struct MediaRendererState
		{
			MediaRendererMuteState muteState;
			boost::uint8_t volume;

			std::string aVTransportURI;
			std::string aVTransportURIMetaData;
			std::string currentTrackURI;
			std::string currentTrackMetaData;

			std::string containerId;

			std::string currentTrackDuration;
			boost::uint32_t currentTrackDurationMS;

			MediaRendererTransportState transportState;

			boost::uint32_t currentTrack;
			boost::uint32_t numberOfTracks;

			std::string contentType;
			boost::uint32_t bitrate;
		};


		struct MediaRendererRoomState
		{
			bool mute;
			boost::uint8_t volume;
			std::string roomUDN;
			MediaRendererTransportState transportState;
		};
		

		typedef boost::signals2::signal<void(const std::string)> typeSignalMediaRendererStateChanged;


		class MediaRenderer : public UPNPDevice
		{
			public:
				MediaRenderer();
				virtual ~MediaRenderer();
				EXPORT virtual bool IsVirtualRenderer();
				EXPORT virtual bool IsRaumfeldRenderer();
				EXPORT virtual MediaRendererState GetMediaRendererState();

				EXPORT void SubscribeSignalMediaRendererStateChanged(const typeSignalMediaRendererStateChanged::slot_type &_subscriber);

				static std::string MediaRendererPlayModeToString(MediaRendererPlayMode _playMode);
				static MediaRendererPlayMode StringToMediaRendererPlayMode(std::string _playModeString);
				static MediaRendererTransportState StringToMediaRendererTransportState(std::string _playModeString);
				static std::string MediaRendererTransportStateToString(MediaRendererTransportState _transportState);
				static std::string MediaRendererMuteStateToString(MediaRendererMuteState _muteState);
				
				virtual void RemoveSubscriptions();	

				EXPORT bool RenderingProxyAvailable(bool _allowCreate = false);
				EXPORT bool ConnectionManagerProxyAvailable(bool _allowCreate = false);
				EXPORT bool AvTransportProxyAvailable(bool _allowCreate = false);

			protected:
				CpProxy		*avTransportProxy;
				CpProxy		*renderingControlProxy;
				CpProxy		*connectionManagerProxy;

				typeSignalMediaRendererStateChanged			signalMediaRendererStateChanged;

				MediaRendererState	mediaRendererState;

				virtual void CreateProxies(bool _allowCreate = true);
				virtual void DeleteProxies();
				virtual void AddSubscriptions();

				boost::mutex mutexProxyObjects;
		
			private:				

		};



		class RaumfeldMediaRenderer : public MediaRenderer
		{
			public:
				RaumfeldMediaRenderer();
				virtual ~RaumfeldMediaRenderer();
				virtual bool IsVirtualRenderer();
				virtual bool IsRaumfeldRenderer();

			protected:
				virtual void CreateProxies(bool _allowCreate = true);

			private:
		};



		class RaumfeldVirtualMediaRenderer : public RaumfeldMediaRenderer
		{
			public:
				RaumfeldVirtualMediaRenderer();
				virtual ~RaumfeldVirtualMediaRenderer();
				EXPORT virtual bool IsVirtualRenderer();
				EXPORT virtual bool IsRaumfeldRenderer();
				
				// play, pause and equivalent commands are only available at 'zone' renderers (virtual ones)				
				EXPORT virtual void Play(bool _sync = true);
				EXPORT virtual void Stop(bool sync = true);
				EXPORT virtual void Pause(bool sync = true);
				EXPORT virtual void Next(bool sync = true);
				EXPORT virtual void Previous(bool sync = true);
				EXPORT virtual void Seek(boost::int32_t _seekToMs, bool _sync = true);
				EXPORT virtual void SetPlayMode(MediaRendererPlayMode _playMode, bool _sync = true);
				EXPORT virtual AvTransportMediaInfo GetMediaInfo();
				EXPORT virtual AvTransportPositionInfo GetPositionInfo();

				// 'volume' commands are present on the rendering service
				// raumfeld virtual renderers (zones) do have commands to set volume and mute on rooms which are attached, so there is no need to
				// call those methods on the room rendere itself?! Therefore we need the room UDN which we have to provide for the method to work							
				EXPORT virtual void SetMute(bool _mute, bool _sync = true);
				EXPORT virtual void SetVolume(boost::uint8_t _volume, bool _sync = true);
				EXPORT virtual void SetRoomMute(std::string _roomUDN, bool _mute, bool _sync = true);
				EXPORT virtual void SetRoomVolume(std::string _roomUDN, boost::uint8_t _volume, bool _sync = true);
				EXPORT virtual void ChangeVolume(boost::int8_t _amount, bool _sync = true);
				
				EXPORT virtual bool GetMute();	
				EXPORT virtual boost::uint8_t GetVolume(); 
				EXPORT virtual bool GetRoomMute(std::string _roomUDN);
				EXPORT virtual boost::uint8_t  GetRoomVolume(std::string _roomUDN);
				
				EXPORT virtual MediaRendererRoomState GetMediaRendererRoomState(std::string _roomUDN);
			
				EXPORT void LoadUri(std::string _uri, bool _play = true);
				EXPORT void LoadSingle(std::string _singleId, bool _play = true);
				EXPORT void LoadContainer(std::string _containerId, boost::int32_t _trackIndex = -1, bool _play = false);	
				EXPORT void PlayTrack(boost::int32_t _trackIndex = -1, bool _sync = true);

				EXPORT MediaItem GetCurrentMediaItem();
											
				virtual void RemoveSubscriptions();

			protected:
				virtual void CreateProxies(bool _allowCreate = true);
				virtual void AddSubscriptions();				

				virtual void BendAVTransportURI(std::string _uri, std::string _uriMetaData, bool _sync = true);
				virtual void SetAVTransportURI(std::string _uri, std::string _uriMetaData, bool _sync = true);

				void BindContainer(std::string _containerId, boost::int32_t _trackIndex = -1, bool _sync = true);
				void SetContainer(std::string _containerId, boost::int32_t _trackIndex = -1, bool _sync = true);

				void OnPlay(IAsync& _aAsync);
				void OnStop(IAsync& _aAsync);
				void OnPause(IAsync& _aAsync);
				void OnNext(IAsync& _aAsync);
				void OnPrevious(IAsync& _aAsync);
				void OnSeek(IAsync& _aAsync);
				void OnSetPlayMode(IAsync& _aAsync);

				void OnGetMediaInfo(IAsync& _aAsync);
				void OnGetPositionInfo(IAsync& _aAsync);

				void OnBendAVTransportURI(IAsync& _aAsync);
				void OnSetAVTransportURI(IAsync& _aAsync);

				void OnSetMute(IAsync& _aAsync);
				void OnSetVolume(IAsync& _aAsync);
				void OnSetRoomMute(IAsync& _aAsync);
				void OnSetRoomVolume(IAsync& _aAsync);
				void OnChangeVolume(IAsync& _aAsync);

				void OnGetMute(IAsync& _aAsync);
				void OnGetVolume(IAsync& _aAsync);
				void OnGetRoomMute(IAsync& _aAsync);
				void OnGetRoomVolume(IAsync& _aAsync);

				void OnBendAvTransportUri(IAsync& _aAsync);

				void OnRenderingServicePropertyChanged();
				void OnAVTransportServicePropertyChanged();
				void OnConnectionManagerPropertyChanged();
				void OnConnectionManagerConnectionIdsChanged();

			private:				
				CpProxyUpnpOrgAVTransport_RaumfeldVirtual1Cpp* GetAvTransportProxy(bool _allowCreate = true);
				CpProxyUpnpOrgRenderingControl_RaumfeldVirtual1Cpp* GetRenderingControlProxy(bool _allowCreate = true);
				CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp* GetConnectionManagerProxy(bool _allowCreate = true);

				boost::mutex mutexRendererState;				

				// this map holds room state struct for rooms which are added to virtual media renderer								
				boost::unordered_map<std::string, MediaRendererRoomState>	roomStateMap;

				// only adds 2 strings together for output of a string ehich includes all properties changed in the 'properties changed' event of the rendere UPNP services
				void AddPropChangeInfo(std::string _propertyName, std::string &_propChangeInfo);
				
				// returns the MediaServer Object
				RaumfeldMediaServer* GetMediaServer();

				// holds the current mediaItem information as MediaItem Class Object
				MediaItem currentMediaItem;

		};

	
}

#endif // RENDERER_H