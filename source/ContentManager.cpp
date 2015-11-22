#include "ContentManager.h"
//#include  "proxies/CpUpnpOrgContentDirectory_Raumfeld1.h"
#include "DeviceManager.h"
#include "ZoneManager.h"
#include "MediaServer.h"


namespace Raumkernel
{

	ContentManager::ContentManager() : HTTPRequestManager()
	{	
	}

	ContentManager::~ContentManager()
	{
	}

	// ContentManager::LockListForUpdate	
	void ContentManager::LockListForUpdate(bool _lock)
	{
		if (_lock)
		{
			mutexListUpdate.lock();
		}
		else
		{
			mutexListUpdate.unlock();
		}

	}

	// ContentManager::Lock	
	void ContentManager::Lock()
	{
		LockListForUpdate(true);
	}

	// ContentManager::UnLock	
	void ContentManager::UnLock()
	{
		LockListForUpdate(false);
	}

	// ContentManager::SubscribeToMediaServer	
	void ContentManager::SubscribeToMediaServer()
	{
		RaumfeldMediaServer* mediaServer = (RaumfeldMediaServer*)managerList.deviceManager->GetMediaServer();
		if (!mediaServer)
			return;

		mediaServer->SubscribeSignalBrowseDone(typeSignalBrowseDone::slot_type(boost::bind(&ContentManager::OnBrowseEnded, this, _1, _2, _3, _4, _5)));
		mediaServer->SubscribeSignalSearchDone(typeSignalSearchDone::slot_type(boost::bind(&ContentManager::OnSearchEnded, this, _1, _2, _3, _4, _5)));

	}

	// ContentManager::SubscribeSignalListContentChanged	
	void ContentManager::SubscribeSignalListContentChanged(const typeSignalListContentChanged::slot_type &_subscriber)
	{
		signalListContentChanged.connect(_subscriber);
	}

	// ContentManager::OnSearchBrowseEnded	
	// search or browse is ended.
	void ContentManager::OnSearchEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData)
	{		
		this->CreateListFromResultXML(_result, _extraData);
	}

	// ContentManager::OnBrowseEnded	
	void ContentManager::OnBrowseEnded(std::string _result, boost::uint32_t _numberReturned, boost::uint32_t _totalMatches, boost::uint32_t _updateId, std::string _extraData)
	{		
		this->CreateListFromResultXML(_result, _extraData);		
	}

	// ContentManager::IsListInCache
	bool ContentManager::IsListInCache(std::string _listId)
	{
		try
		{
			auto it = listCache.find(_listId);
			if (it != listCache.end())
				return true;
		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
		}

		return false;
	}


	// ContentManager::SetLastUpdateIdForList	
	void ContentManager::SetLastUpdateIdForList(std::string _listId, boost::uint32_t _updateId)
	{	
		listUpdateId.erase(_listId);
		listUpdateId.insert(std::make_pair(_listId, _updateId));
	}

	// ContentManager::GetLastUpdateIdForList	
	boost::uint32_t ContentManager::GetLastUpdateIdForList(std::string _listId)
	{
		auto mapItem = listUpdateId.find(_listId);
		if (mapItem != listUpdateId.end())
			return mapItem->second;		
		return 0;
	}

	// ContentManager::ListReady	
	void ContentManager::ListReady(std::string _listId, bool _zonePlaylist)
	{
		// each update of a list we generate and store a random number
		// this is needed for JSON webserver for long polling actions
		boost::uint32_t lastUpdateId = this->GetLastUpdateIdForList(_listId);
		boost::uint32_t curUpdateId = Utils::GetRandomNumber();
		if (lastUpdateId == curUpdateId)
			lastUpdateId++;
		else
			lastUpdateId = curUpdateId;
		this->SetLastUpdateIdForList(_listId, lastUpdateId);

		// if its a zone list id store global update id for zone
		if (_zonePlaylist)
		{
			lastUpdateId = this->GetLastUpdateIdForList(ALLZONEPLAYLISTSUPDATEID);
			curUpdateId = Utils::GetRandomNumber();
			if (lastUpdateId == curUpdateId)
				lastUpdateId++;
			else
				lastUpdateId = curUpdateId;
			this->SetLastUpdateIdForList(ALLZONEPLAYLISTSUPDATEID, lastUpdateId);
		}

		signalListContentChanged(_listId);
	}

	// ContentManager::StartGetMediaItemListByZoneUDN
	// EG::  0/Zones/uuid%3Ad225b9e5-6787-4421-b776-e31b142591ef,42571234		
	void ContentManager::StartGetMediaItemListsByContainerUpdateIds(std::string _containerUpdateIds, bool _sync)
	{
		std::string containerUpdateId = "", listId = "";

		this->Log(LogType::LOGDEBUG, "Container UpdateID's changed: " + _containerUpdateIds, __FUNCTION__);
		
		try
		{
			auto containerUpdateIdVector = Utils::ExplodeString(_containerUpdateIds, ",");
			for (auto &containerUpdateId : containerUpdateIdVector)
			{			
				// we only search for zone containers and do update the zone container list, all other lists do not bother us for now
				// zone containers starts with "0/Zones/" and ends with ZoneUDN				
				if (containerUpdateId.find("0/Zones/") != std::string::npos)
				{
					listId = containerUpdateId.substr(8, containerUpdateId.length() - 8);
					listId = Utils::Unescape(listId);
					this->StartGetMediaItemListByContainerId(containerUpdateId, "", listId, true);
				}

				// TODO: Cover LastPlayed and Favourites List
				
			}

		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
		}
		
	}


	// ContentManager::StartGetMediaItemListByZoneUDN	
	void ContentManager::StartGetMediaItemListByZoneUDN(std::string _zoneUDN, bool _sync)
	{
		std::string containerId = "", listId = _zoneUDN;
		RaumfeldVirtualMediaRenderer *mediaRenderer;

		try
		{
			mediaRenderer = (RaumfeldVirtualMediaRenderer*)managerList.deviceManager->GetMediaRenderer(Utils::FormatUDN(_zoneUDN));
			if (mediaRenderer)
			{

				containerId = mediaRenderer->GetMediaRendererState().containerId;
				if (!containerId.empty())
				{
					this->StartGetMediaItemListByContainerId(containerId, "", listId, true, _sync);
				}
				else
				{
					// no container? then it means we do have only one item in list! that means the current item on the MediaRenderer is the list!
					// TODO: @@@
					this->Lock();

					std::list<MediaItem> newList;
					
					MediaItem mediaItem = mediaRenderer->GetCurrentMediaItem();
					if (!mediaItem.id.empty())
					{
						newList.push_back(mediaItem);

						auto it = listCache.find(listId);
						if (it != listCache.end())
							listCache.erase(listId);
						listCache.insert(std::make_pair(listId, newList));
					}

					this->UnLock();		

					this->ListReady(listId, true);
				}
			}

		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
		}
	}
	

	
	// ContentManager::StartGetMediaItemListByContainerId	
	// loads a list from a container
	void ContentManager::StartGetMediaItemListByContainerId(std::string _containerId, std::string _searchCriteria, std::string _listId, bool _isZonePlaylist, bool _sync)
	{
		RaumfeldMediaServer* mediaServer = (RaumfeldMediaServer*)managerList.deviceManager->GetMediaServer();	
		if (!mediaServer)
			return;	

		this->Log(LogType::LOGDEBUG, "Get MediaList: " + _listId + "  for container: " + _containerId, __FUNCTION__);

		std::string zonePlaylistMarker = _isZonePlaylist ? "&zonePlaylist" : "";

		if (_listId.empty())
		{
			_listId = _containerId;
			if (!_searchCriteria.empty())
				_listId += ":" + _searchCriteria;
		}

		if (!_searchCriteria.empty())
		{			
			mediaServer->Search(_containerId, _searchCriteria, "listId=" + _listId + zonePlaylistMarker, _sync);
		}
		else
		{
			mediaServer->Browse(_containerId, MediaServerBrowseFlag::MSBF_BrowseDirectChildren, "listId=" + _listId + zonePlaylistMarker, _sync);
		}
		
	}

	// ContentManager::EndGetMediaItemList
	std::list<MediaItem> ContentManager::EndGetMediaItemList(std::string _listId)
	{
		std::list<MediaItem> newList;

		boost::mutex::scoped_lock scoped_lock(mutexListUpdate);

		try
		{
			auto it = listCache.find(_listId);
			if (it != listCache.end())
				return it->second;		

		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			_listId = "";
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
			_listId = "";
		}

		return newList;
	}
	

	// ContentManager::CreateListFromResultXML
	void ContentManager::CreateListFromResultXML(std::string _result, std::string _extraData)
	{
		xml_document<> doc;
		xml_node<> *itemNode, *rootNode;
		std::list<MediaItem> newList;
		std::string listId;		
		bool isZoneLPlaylist = false;


		try
		{

			auto map = Utils::ParseQueryString(_extraData);
			auto itXD = map.find("listid");
			if (itXD != map.end())
				listId = itXD->second;
			itXD = map.find("zoneplaylist");
			if (itXD != map.end())
				isZoneLPlaylist = true;
		
			// to parse the string we have to put it into char* (because c_str() returns const char*)
			char* cstr = new char[_result.size() + 1];
			strcpy(cstr, _result.c_str());
			doc.parse<0>(cstr);

			// find the root node which has to be the 'DIDL-Lite' node	
			rootNode = doc.first_node("DIDL-Lite", 0, false);
			if (!rootNode)
			{
				this->Log(LogType::LOGERROR, "Result XML from device does not contain DIDL-Lite block!", __FUNCTION__);
				return;
			}
	
			// run through items
			itemNode = rootNode->first_node("item", 0, false);
			while (itemNode)
			{			
				MediaItem mediaItem = this->CreateMediaItemFromXMLNode(itemNode);								
				newList.push_back(mediaItem);
				itemNode = itemNode->next_sibling("item", 0, false);
			}

			// run through container
			itemNode = rootNode->first_node("container", 0, false);
			while (itemNode)
			{
				MediaItem mediaItem = this->CreateMediaItemFromXMLNode(itemNode);
				newList.push_back(mediaItem);
				itemNode = itemNode->next_sibling("container", 0, false);
			}

			// Lock main list and copy in new created list!
			this->Lock(); // TODO: @@@@ problem!!!! HMMMMM.....

			auto it = listCache.find(listId);
			if (it != listCache.end())
				listCache.erase(listId);
			
			listCache.insert(std::make_pair(listId, newList));

		}		
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			listId = "";
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
			listId = "";
		}

		this->UnLock();

		this->ListReady(listId, isZoneLPlaylist);
	}


	// ContentManager::CreateMediaItemFromXMLNode
	MediaItem ContentManager::CreateMediaItemFromXMLNode(xml_node<> *_xmlNode)
	{
		xml_node<> *itemNode = _xmlNode, *valueNode;
		xml_attribute<> *attribute;
		MediaItem mediaItem;

		try
		{
							
			attribute = itemNode->first_attribute("id", 0, false);
			if (attribute)
				mediaItem.id = attribute->value();

			attribute = itemNode->first_attribute("parentId", 0, false);
			if (attribute)
				mediaItem.parentId = attribute->value();

			valueNode = itemNode->first_node("raumfeld:name", 0, false);
			if (valueNode)
				mediaItem.raumfeldName = valueNode->value();

			valueNode = itemNode->first_node("dc:title", 0, false);
			if (valueNode)
				mediaItem.title = valueNode->value();

			valueNode = itemNode->first_node("upnp:class", 0, false);
			if (valueNode)
				mediaItem.upnpClass = valueNode->value();

			valueNode = itemNode->first_node("raumfeld:section", 0, false);
			if (valueNode)
				mediaItem.raumfeldSection = valueNode->value();

			valueNode = itemNode->first_node("res", 0, false);
			if (valueNode)
			{
				mediaItem.res = valueNode->value();
				attribute = valueNode->first_attribute("duration", 0, false);
				if (attribute)
					mediaItem.duration = attribute->value();
			}

			// ALBUM
			valueNode = itemNode->first_node("upnp:album", 0, false);
			if (valueNode)
				mediaItem.album = valueNode->value();

			valueNode = itemNode->first_node("upnp:albumArtUri", 0, false);
			if (valueNode)
				mediaItem.albumArtUri = valueNode->value();

			// ARTIST
			valueNode = itemNode->first_node("upnp:artist", 0, false);
			if (valueNode)
				mediaItem.artist = valueNode->value();

			valueNode = itemNode->first_node("upnp:artistArtUri", 0, false);
			if (valueNode)
				mediaItem.artistArtUri = valueNode->value();
					

		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);		
		}

		return mediaItem;
	
	}


	// ContentManager::CreateMediaItemFromCurrentTrackMetadata
	MediaItem ContentManager::CreateMediaItemFromCurrentTrackMetadata(std::string _trackMetadata)
	{
		xml_document<> doc;
		xml_node<> *itemNode, *rootNode;
		std::list<MediaItem> newList;
		MediaItem mediaItem;

		try
		{

			// to parse the string we have to put it into char* (because c_str() returns const char*)
			char* cstr = new char[_trackMetadata.size() + 1];
			strcpy(cstr, _trackMetadata.c_str());
			doc.parse<0>(cstr);

			// find the root node which has to be the 'DIDL-Lite' node	
			rootNode = doc.first_node("DIDL-Lite", 0, false);
			if (!rootNode)
			{
				this->Log(LogType::LOGERROR, "Result XML from device does not contain DIDL-Lite block!", __FUNCTION__);
				return mediaItem;
			}

			// run through items
			itemNode = rootNode->first_node("item", 0, false);
			if (itemNode)
				mediaItem = this->CreateMediaItemFromXMLNode(itemNode);					

		}
		catch (std::exception &ex)
		{
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
		}
		catch (...)
		{
			this->Log(LogType::LOGERROR, "Unknown Error", __FUNCTION__);
		}

		return mediaItem;
	
	}

}

