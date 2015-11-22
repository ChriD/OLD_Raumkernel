#include "MediaServer.h"
#include "ContentManager.h"


namespace Raumkernel
{
	
		MediaServer::MediaServer() : UPNPDevice()
		{
			contentDirectoryProxy = nullptr;
			connectionManagerProxy = nullptr;
		}

		MediaServer::~MediaServer()
		{
			// no need to remove subscriptions! delete of proxies will handle this correct!
			this->DeleteProxies();
		}

		bool MediaServer::IsRaumfeldMediaServer()
		{
			return false;
		}

		void MediaServer::DeleteProxies()
		{
			if (contentDirectoryProxy)
			{
				delete contentDirectoryProxy;
				contentDirectoryProxy = nullptr;
			}

			if (connectionManagerProxy)
			{
				delete connectionManagerProxy;
				connectionManagerProxy = nullptr;
			}
		}



		RaumfeldMediaServer::RaumfeldMediaServer() : MediaServer()
		{
		}

		RaumfeldMediaServer::~RaumfeldMediaServer()
		{
			// no need to remove subscriptions! delete of proxies will handle this correct!
			this->DeleteProxies();
		}

		bool RaumfeldMediaServer::IsRaumfeldMediaServer()
		{
			return true;
		}

		void RaumfeldMediaServer::CreateProxies(bool _allowCreate)
		{
			if (!cpDevice)
			{
				this->Log(LogType::LOGWARNING, "Calling 'CreateProxies' on MediaServer '" + this->GetIdentificationString() + "' without having CP-Device!", __FUNCTION__);
				this->DeleteProxies();		
				return;
			}

			if (!_allowCreate)
				return;

			if (!connectionManagerProxy || !contentDirectoryProxy)
				this->Log(LogType::LOGDEBUG, "Create proxies for MediaServer '" + this->GetIdentificationString() + "'", __FUNCTION__);
			
			//if (!contentDirectoryProxy)		contentDirectoryProxy	= new CpProxyUpnpOrgContentDirectory_Raumfeld1Cpp(*cpDevice);
			if (!contentDirectoryProxy)		contentDirectoryProxy = new CpProxyUpnpOrgContentDirectory1Cpp(*cpDevice);
			if (!connectionManagerProxy)	connectionManagerProxy	= new CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp(*cpDevice);
			// INFO: Add more proxies here if necesarry
		}

		void RaumfeldMediaServer::AddSubscriptions()
		{
			this->Log(LogType::LOGDEBUG, "Adding subscriptions for MediaServer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			OpenHome::Functor functorCD = OpenHome::MakeFunctor(*this, &RaumfeldMediaServer::OnContentDirectoryPropertyChanged);
			OpenHome::Functor functorCDContainer = OpenHome::MakeFunctor(*this, &RaumfeldMediaServer::OnContentDirectoryContainerUpdateIdsChanged);
			OpenHome::Functor functorCDSystemContainer = OpenHome::MakeFunctor(*this, &RaumfeldMediaServer::OnContentDirectorySystemUpdateIdChanged);
			OpenHome::Functor functorCM = OpenHome::MakeFunctor(*this, &RaumfeldMediaServer::OnConnectionManagerPropertyChanged);
			OpenHome::Functor functorCMConIds = OpenHome::MakeFunctor(*this, &RaumfeldMediaServer::OnConnectionManagerConnectionIdsChanged);

			// TODO: Disabled for now. Some exceptions were thrown and until i need this subscriptions they will be desactivated!
			//this->GetContentDirectoryProxy()->SetPropertyChanged(functorCD);
			//this->GetContentDirectoryProxy()->SetPropertyInitialEvent(functorCD);
			this->GetContentDirectoryProxy()->SetPropertyContainerUpdateIDsChanged(functorCDContainer);
			//this->GetContentDirectoryProxy()->SetPropertySystemUpdateIDChanged(functorCDSystemContainer);
			this->GetContentDirectoryProxy()->Subscribe();

			//this->GetConnectionManagerProxy()->SetPropertyChanged(functorCM);
			//this->GetConnectionManagerProxy()->SetPropertyInitialEvent(functorCM);
			//this->GetConnectionManagerProxy()->SetPropertyCurrentConnectionIDsChanged(functorCMConIds);
			//this->GetConnectionManagerProxy()->Subscribe();
		}

		void RaumfeldMediaServer::OnConnectionManagerConnectionIdsChanged()
		{
			std::string propertyXML = "";

			this->Log(LogType::LOGDEBUG, "ConnectionManager on '" + this->GetIdentificationString() + "': Connection Id's changed!", __FUNCTION__);

			this->GetConnectionManagerProxy()->PropertyCurrentConnectionIDs(propertyXML);
		}

		void RaumfeldMediaServer::OnContentDirectoryContainerUpdateIdsChanged()
		{
			std::string updateIds = "";

			this->Log(LogType::LOGDEBUG, "ContentDirectory on '" + this->GetIdentificationString() + "': Container Id's changed!", __FUNCTION__);

			this->GetContentDirectoryProxy()->PropertyContainerUpdateIDs(updateIds);
			
			managerList.contentManager->StartGetMediaItemListsByContainerUpdateIds(updateIds);
		}

		void RaumfeldMediaServer::OnContentDirectorySystemUpdateIdChanged()
		{
			boost::uint32_t systemUpdateId;

			this->Log(LogType::LOGDEBUG, "ContentDirectory on '" + this->GetIdentificationString() + "': System Update Id changed!", __FUNCTION__);

			this->GetContentDirectoryProxy()->PropertySystemUpdateID(systemUpdateId);
		}


		void RaumfeldMediaServer::OnConnectionManagerPropertyChanged()
		{
			std::string propertyXML = "";

			this->Log(LogType::LOGDEBUG, "ConnectionManager on '" + this->GetIdentificationString() + "': property changed!", __FUNCTION__);
			// no properties????
		}

		void RaumfeldMediaServer::OnContentDirectoryPropertyChanged()
		{
			std::string propertyXML = "";

			this->Log(LogType::LOGDEBUG, "ContentDirectory on '" + this->GetIdentificationString() + "': property changed!", __FUNCTION__);
			// no properties????
		}

		void RaumfeldMediaServer::RemoveSubscriptions()
		{
			// use this method to add subscriptions to upnp services
			this->Log(LogType::LOGDEBUG, "Removing subscriptions for MediaServer '" + this->GetIdentificationString() + "'", __FUNCTION__);

			if (contentDirectoryProxy) contentDirectoryProxy->Unsubscribe();
			if (connectionManagerProxy) connectionManagerProxy->Unsubscribe();
		}

		CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp* RaumfeldMediaServer::GetConnectionManagerProxy()
		{
			this->CreateProxies();
			return (CpProxyUpnpOrgConnectionManager_RaumfeldVirtual1Cpp*)connectionManagerProxy;
		}

		//CpProxyUpnpOrgContentDirectory_Raumfeld1Cpp* RaumfeldMediaServer::GetContentDirectoryProxy()
		CpProxyUpnpOrgContentDirectory1Cpp* RaumfeldMediaServer::GetContentDirectoryProxy()
		{
			this->CreateProxies();
			//return (CpProxyUpnpOrgContentDirectory_Raumfeld1Cpp*)contentDirectoryProxy;
			return (CpProxyUpnpOrgContentDirectory1Cpp*)contentDirectoryProxy;
		}


		std::string RaumfeldMediaServer::CreateAVTransportUri_Container(std::string _containerId, boost::int32_t _trackIndex)
		{
			std::string uri;
			std::string containerIdUri = _containerId;

			uri = Utils::EncodeUriPart(this->GetUDN()) + "?sid=" + Utils::EncodeUriPart("urn:upnp-org:serviceId:ContentDirectory") + "&cid=" + Utils::EncodeUriPart(containerIdUri) + "&md=0";

			if (_trackIndex >= 0)
				uri += "&fii=" + Utils::EncodeUriPart(std::to_string(_trackIndex));

			uri = "dlna-playcontainer://" + uri;
			
			// a valid transport uri looks like this!
			//dlna-playcontainer://uuid%3Aed3bd3db-17b1-4dbe-82df-5201c78e632c?sid=urn%3Aupnp-org%3AserviceId%3AContentDirectory&cid=0%2FPlaylists%2FMyPlaylists%2FTest&md=0	
            return uri;
		}


		std::string RaumfeldMediaServer::CreateAVTransportUri_Single(std::string _singleId)
		{
			std::string uri;
			std::string singleIdUri = _singleId;

			uri = Utils::EncodeUriPart(this->GetUDN()) + "?sid=" + Utils::EncodeUriPart("urn:upnp-org:serviceId:ContentDirectory") + "&iid=" + Utils::EncodeUriPart(_singleId);
			

			uri = "dlna-playsingle://" + uri;

			// a valid transport uri looks like this!
			//dlna-playsingle://uuid%3Aed3bd3db-17b1-4dbe-82df-5201c78e632c?sid=urn%3Aupnp-org%3AserviceId%3AContentDirectory&iid=0%2FRadioTime%2FLocalRadio%2Fs-s68932
			return uri;
		}
		

		bool RaumfeldMediaServer::ContentDirectoryProxyAvailable(bool _allowCreate)
		{
			if (!cpDevice)
				return false;			

			this->CreateProxies(_allowCreate);
			if (contentDirectoryProxy == nullptr)
			{
				this->Log(LogType::LOGWARNING, "Proxy Instance not available!" + this->GetIdentificationString(), __FUNCTION__);
				return false;
			}
			return true;
		}


		// RaumfeldMediaServer::Search
		// due to the fact that the UPNP Stack doesnt support lamdas and we ot no idea which sink called the callbak we do make a thread with async action°		
		void RaumfeldMediaServer::Search(std::string _containerId, std::string _searchCriteria, std::string _extraData, bool _sync)
		{
			boost::thread		thread;

			thread = boost::thread(&RaumfeldMediaServer::SearchThread, this, _containerId, _searchCriteria, _extraData);
			if (_sync)
				thread.join();
		}


		// RaumfeldMediaServer::SearchThread
		void RaumfeldMediaServer::SearchThread(std::string _containerId, std::string _searchCriteria, std::string _extraData)
		{
			std::string	result = "";
			boost::uint32_t numberReturned = 0, totalMatches = 0, updateId = 0;

			if (!this->ContentDirectoryProxyAvailable())
				return;				

			try
			{			
				this->Log(LogType::LOGDEBUG, "Search ContentDirectory for " + _containerId + " : " + _searchCriteria, __FUNCTION__);

				this->GetContentDirectoryProxy()->SyncSearch(_containerId, _searchCriteria, "*", 0, 0, "", result, numberReturned, totalMatches, updateId);
				this->SearchEnded(result, numberReturned, totalMatches, updateId, _extraData);					
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
				this->Log(LogType::LOGERROR, "Unknown error", __FUNCTION__);
			}							
			
		}

		// RaumfeldMediaServer::Browse
		// due to the fact that the UPNP Stack doesnt support lamdas and we ot no idea which sink called the callbak we do make a thread with async action°		
		void RaumfeldMediaServer::Browse(std::string _containerId, MediaServerBrowseFlag _browseFlag, std::string _extraData, bool _sync)
		{
			boost::thread		thread;
										
			thread = boost::thread(&RaumfeldMediaServer::BrowseThread, this, _containerId, _browseFlag, _extraData);
			if (_sync)
				thread.join();
		}


		void RaumfeldMediaServer::BrowseThread(std::string _containerId, MediaServerBrowseFlag _browseFlag, std::string _extraData)
		{
			std::string	result = "", browseFlag = "BrowseDirectChildren";
			boost::uint32_t numberReturned = 0, totalMatches = 0, updateId = 0;

			if (!this->ContentDirectoryProxyAvailable())
				return;			

			try
			{

				if (_browseFlag == MediaServerBrowseFlag::MSBF_BrowseMetadata)
					browseFlag = "BrowseMetadata";			

				this->Log(LogType::LOGDEBUG, "Browse ContentDirectory for " + _containerId + " : " + browseFlag, __FUNCTION__);

				this->GetContentDirectoryProxy()->SyncBrowse(_containerId, browseFlag, "*", 0, 0, "", result, numberReturned, totalMatches, updateId);
				this->BrowseEnded(result, numberReturned, totalMatches, updateId, _extraData);
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
				this->Log(LogType::LOGERROR, "Unknown error", __FUNCTION__);
			}

		}


		void RaumfeldMediaServer::SearchEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData)
		{
			signalSearchDone(_result, _numberReturned, _totalMatches, _updateId, _extraData);
		}

		void RaumfeldMediaServer::BrowseEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData)
		{
			signalBrowseDone(_result, _numberReturned, _totalMatches, _updateId, _extraData);
		}


		// OBSOLETE
		void RaumfeldMediaServer::OnSearchEnd(IAsync& _aAsync)
		{
			std::string	result;
			boost::uint32_t numberReturned, totalMatches, updateId;
									
			try
			{
				if (!this->ContentDirectoryProxyAvailable())
					return;
				
				this->Log(LogType::LOGDEBUG, "ContentDirectory on '" + this->GetIdentificationString() + "': search end!", __FUNCTION__);

				this->GetContentDirectoryProxy()->EndSearch(_aAsync, result, numberReturned, totalMatches, updateId);
				this->SearchEnded(result, numberReturned, totalMatches, updateId, "");
				
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
				this->Log(LogType::LOGERROR, "Unknown error", __FUNCTION__);
			}
		}

		// OBSOLETE
		void RaumfeldMediaServer::OnBrowseEnd(IAsync& _aAsync)
		{
			std::string	result = "";
			boost::uint32_t numberReturned = 0, totalMatches = 0, updateId = 0;

			try
			{
				if (!this->ContentDirectoryProxyAvailable())
					return;

				this->Log(LogType::LOGDEBUG, "ContentDirectory on '" + this->GetIdentificationString() + "': browse end!", __FUNCTION__);				

				this->GetContentDirectoryProxy()->EndBrowse(_aAsync, result, numberReturned, totalMatches, updateId);
				this->BrowseEnded(result, numberReturned, totalMatches, updateId, "");

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
				this->Log(LogType::LOGERROR, "Unknown error", __FUNCTION__);
			}
		}
		


		void RaumfeldMediaServer::SubscribeSignalBrowseDone(const typeSignalBrowseDone::slot_type &_subscriber)
		{
			signalBrowseDone.connect(_subscriber);
		}

		void RaumfeldMediaServer::SubscribeSignalSearchDone(const typeSignalSearchDone::slot_type &_subscriber)
		{
			signalSearchDone.connect(_subscriber);
		}


		// when storing into the map we have to be sure we do have exculsive access!
		//boost::mutex::scoped_lock scoped_lock(mutexListUpdate);


		/*
		// retrieve list for a containerId
		public void retrieveListByContainerId(String _containerId, String _searchCriteria = "", Boolean _syncRequest = false)
		{
		System.String result;
		uint numberReturned, totalMatches, updateID;

		containerId = _containerId;

		if (_syncRequest)
		{
		if (!String.IsNullOrWhiteSpace(_searchCriteria))
		{
		if (_searchCriteria == "*") _searchCriteria = "";
		contentDirectory.SearchSync(containerId, _searchCriteria, "*", 0, 0, String.Empty, out result, out numberReturned, out totalMatches, out updateID);
		this.contentDirectory_SearchSink(contentDirectory, containerId, String.Empty, "*", 0, 0, String.Empty, result, numberReturned, totalMatches, updateID, null, null);
		}
		else
		{
		contentDirectory.BrowseSync(_containerId, "*", 0, 0, String.Empty, out result, out numberReturned, out totalMatches, out updateID);
		this.contentDirectory_BrowseSink(contentDirectory, _containerId, "*", 0, 0, String.Empty, result, numberReturned, totalMatches, updateID, null, null);
		}
		}
		else
		{
		if (!String.IsNullOrWhiteSpace(_searchCriteria))
		{
		if (_searchCriteria == "*") _searchCriteria = "";
		contentDirectory.Search(containerId, _searchCriteria, "*", 0, 0, String.Empty, containerId, contentDirectory_SearchSink);
		}
		else
		contentDirectory.Browse(_containerId, "*", 0, 0, String.Empty, containerId, contentDirectory_BrowseSink);

		}

		}*/




		// when storing into the map we have to be sure we do have exculsive access!
		//boost::mutex::scoped_lock scoped_lock(mutexListUpdate);


		/*
		// retrieve list for a containerId
		public void retrieveListByContainerId(String _containerId, String _searchCriteria = "", Boolean _syncRequest = false)
		{
		System.String result;
		uint numberReturned, totalMatches, updateID;

		containerId = _containerId;

		if (_syncRequest)
		{
		if (!String.IsNullOrWhiteSpace(_searchCriteria))
		{
		if (_searchCriteria == "*") _searchCriteria = "";
		contentDirectory.SearchSync(containerId, _searchCriteria, "*", 0, 0, String.Empty, out result, out numberReturned, out totalMatches, out updateID);
		this.contentDirectory_SearchSink(contentDirectory, containerId, String.Empty, "*", 0, 0, String.Empty, result, numberReturned, totalMatches, updateID, null, null);
		}
		else
		{
		contentDirectory.BrowseSync(_containerId, "*", 0, 0, String.Empty, out result, out numberReturned, out totalMatches, out updateID);
		this.contentDirectory_BrowseSink(contentDirectory, _containerId, "*", 0, 0, String.Empty, result, numberReturned, totalMatches, updateID, null, null);
		}
		}
		else
		{
		if (!String.IsNullOrWhiteSpace(_searchCriteria))
		{
		if (_searchCriteria == "*") _searchCriteria = "";
		contentDirectory.Search(containerId, _searchCriteria, "*", 0, 0, String.Empty, containerId, contentDirectory_SearchSink);
		}
		else
		contentDirectory.Browse(_containerId, "*", 0, 0, String.Empty, containerId, contentDirectory_BrowseSink);

		}

		}*/
}