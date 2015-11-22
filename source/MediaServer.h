#ifndef MEDIASERVER_H
#define MEDIASERVER_H


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

#include  "proxies/CpUpnpOrgContentDirectory1.h"
#include  "proxies/CpUpnpOrgContentDirectory_Raumfeld1.h"
#include  "proxies/CpUpnpOrgConnectionManager_RaumfeldVirtual1.h"


namespace Raumkernel
{

	enum class MediaServerBrowseFlag { MSBF_BrowseDirectChildren, MSBF_BrowseMetadata};

	typedef boost::signals2::signal<void(const std::string, boost::uint32_t, boost::uint32_t, boost::uint32_t, std::string)> typeSignalBrowseDone;
	typedef boost::signals2::signal<void(const std::string, boost::uint32_t, boost::uint32_t, boost::uint32_t, std::string)> typeSignalSearchDone;


	class MediaServer : public UPNPDevice
	{
		public:
			MediaServer();
			virtual ~MediaServer();		
			virtual bool IsRaumfeldMediaServer();				

		protected:
			CpProxy		*contentDirectoryProxy;			
			CpProxy		*connectionManagerProxy;	

			virtual void DeleteProxies();

		private:
	};



	class RaumfeldMediaServer : public MediaServer
	{
		public:
			RaumfeldMediaServer();
			virtual ~RaumfeldMediaServer();
			virtual bool IsRaumfeldMediaServer();

			virtual void AddSubscriptions();
			virtual void RemoveSubscriptions();

			EXPORT bool ContentDirectoryProxyAvailable(bool _allowCreate = false);

			EXPORT void SubscribeSignalBrowseDone(const typeSignalBrowseDone::slot_type &_subscriber);
			EXPORT void SubscribeSignalSearchDone(const typeSignalSearchDone::slot_type &_subscriber);

			// content directory methods
			EXPORT void Search(std::string _containerId, std::string _searchCriteria = "", std::string _extraData = "", bool _sync = false);
			EXPORT void Browse(std::string _containerId, MediaServerBrowseFlag _browseFlag = MediaServerBrowseFlag::MSBF_BrowseDirectChildren, std::string _extraData = "", bool _sync = false);

			// this methods creates a transport uri string for setting or bending AVTransportURI
			std::string CreateAVTransportUri_Container(std::string _containerId, boost::int32_t _trackIndex = -1);
			std::string CreateAVTransportUri_Single(std::string _singleId);

		protected:
			//CpProxyUpnpOrgContentDirectory_Raumfeld1Cpp* GetContentDirectoryProxy();
			CpProxyUpnpOrgContentDirectory1Cpp* GetContentDirectoryProxy();
			CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp* GetConnectionManagerProxy();

			virtual void CreateProxies(bool _allowCreate = true);	

			// following "..Ended" methos will be called if sync or async action is done
			void SearchEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData);
			void BrowseEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData);

			void SearchThread(std::string _containerId, std::string _searchCriteria = "", std::string _extraData = "");
			void BrowseThread(std::string _containerId, MediaServerBrowseFlag _browseFlag = MediaServerBrowseFlag::MSBF_BrowseDirectChildren, std::string _extraData = "");

			typeSignalBrowseDone			signalBrowseDone;
			typeSignalSearchDone			signalSearchDone;

		private:
			void OnConnectionManagerPropertyChanged();
			void OnConnectionManagerConnectionIdsChanged();
			void OnContentDirectoryPropertyChanged();
			void OnContentDirectoryContainerUpdateIdsChanged();
			void OnContentDirectorySystemUpdateIdChanged();

			void OnSearchEnd(IAsync& _aAsync);
			void OnBrowseEnd(IAsync& _aAsync);
	};

	
}


#endif // MEDIASERVER_H