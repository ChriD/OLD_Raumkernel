#include "WebServerResponseHandler.h"

#include "ContentManager.h"
#include "ZoneManager.h"
#include "DeviceManager.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <fstream>

namespace Raumkernel
{

	ResponseHandler::ResponseHandler() : RaumkernObject()
	{
		cancelRequests = false;
	}

	ResponseHandler::~ResponseHandler()
	{
	}


	void ResponseHandler::SetCancelRequets(bool _cancel)
	{
		cancelRequests = _cancel;
	}

	std::string ResponseHandler::GetQueryValue(std::string _uri, std::string _key)
	{		
		boost::network::uri::uri instance(_uri);
		std::string uriQuery = instance.query();

		// parse the query string for all id and value pairs it may have
		boost::unordered_map<std::string, std::string> queryValues = Utils::ParseQueryString(uriQuery);

		// now handle the cid(containerId) information. if its there, then get it and store it
		boost::unordered_map<std::string, std::string>::iterator it = queryValues.find(_key);
		if (it != queryValues.end())		
			return it->second;
		return "";
	}


	JSONResponseHandler::JSONResponseHandler() : ResponseHandler()
	{
	}

	JSONResponseHandler::~JSONResponseHandler()
	{
	}

	ResponseHandlerResponseReturn JSONResponseHandler::CreateResponseForRequest(std::string _requestPath, std::unordered_map<std::string, std::string> _header, std::string _content)
	{
		std::vector<std::string> urlVector, actionAndQueryVector;
		std::string actionAndQuery = "", urlQuery = "", action = "", updateId = "", sessionId= "";

		// vector positions of "raumserver" and "json" are always present! (we insured that with the regex values on server start)
		urlVector = Utils::ExplodeString(_requestPath, "/");
		if (urlVector.size() < 4)
		{
			this->Log(LogType::LOGERROR, "PATH For Request not correct!", __FUNCTION__);
			throw std::runtime_error("PATH For Request not correct! '" + _requestPath + "'");
		}

		actionAndQuery = Utils::Unescape(urlVector[3]);
		actionAndQueryVector = Utils::ExplodeString(actionAndQuery, "?");
		if (actionAndQueryVector.size() > 1)
			urlQuery = actionAndQueryVector[1];
		action = actionAndQueryVector[0];
		boost::algorithm::to_lower(action);

		// search for updateID
		auto it = _header.find("updateID");
		if (it != _header.end())
			updateId = it->second;

		// search for sessionID
		it = _header.find("sessionID");
		if (it != _header.end())
			sessionId = it->second;	

		if (action == ACTION_JSON_GETROOMLIST)
		{			
			return this->ResponseRoomList(updateId, sessionId);
		}
		else if (action == ACTION_JSON_GETZONELIST)
		{
			return this->ResponseZoneList(updateId, sessionId);
		}
		else if (action == ACTION_JSON_GETRENDERERSTATE)
		{
			return this->ResponseMediaRendererState(updateId, sessionId);
		}
		else if (action == ACTION_JSON_GETZONEPLAYLIST)
		{
			std::string zoneUDN = this->GetQueryValue("http://dummy.com/" + _requestPath, "zoneudn");			
			return this->ResponseZonePlaylist(zoneUDN, updateId, sessionId);
		}
		else if (action == ACTION_JSON_GETCONTAINERLIST)
		{
			bool useCache = false;
			std::string containerId = this->GetQueryValue("http://dummy.com/" + _requestPath, "cid");
			if (containerId.empty())
				throw std::runtime_error("List id has to be defined!");			
			std::string useCacheString = this->GetQueryValue("http://dummy.com/" + _requestPath, "usecache");
			if (!useCacheString.empty() && useCacheString == "1")
				useCache = true;	
			return this->ResponseContainerList(containerId, useCache);
		}
		else if (action == ACTION_ABORTREQUESTS)
		{
			std::string id = this->GetQueryValue("http://dummy.com/" + _requestPath, "id");
			return this->AbortRequests(id);
		}
		else
		{
			throw std::runtime_error("JSON Request Action '" + action + "' not defined!");
		}

	}

	ResponseHandlerResponseReturn JSONResponseHandler::AbortRequests(std::string _id)
	{
		abortRequestListMutex.lock();
		try
		{
			abortRequestIdList.push_back(_id);
		}
		catch (...)
		{
			throw std::runtime_error("Aborting requests failed! '");
		}
		abortRequestListMutex.unlock();

		// Wait a little bit to let server threads have aknowledged the shutdown, then remove id from list
		this->Sleep(1000);

		abortRequestListMutex.lock();
		try
		{			
			abortRequestIdList.remove(_id);
		}
		catch (...)
		{
			throw std::runtime_error("removing abort id from list failed! '");
		}
		abortRequestListMutex.unlock();

		ResponseHandlerResponseReturn responseReturn;
		responseReturn.content = "requests aborted";

		return responseReturn;

	}

	std::string JSONResponseHandler::CreateMediaItemObjectJSONString(MediaItem _mediaItem)
	{		
		std::string jsonResponse;

		jsonResponse += "{";				
		jsonResponse += "\"title\": \"" + Utils::EncodeValue(_mediaItem.title) + "\", ";
		jsonResponse += "\"album\": \"" + Utils::EncodeValue(_mediaItem.album) + "\", ";
		jsonResponse += "\"artist\": \"" + Utils::EncodeValue(_mediaItem.artist) + "\", ";
		jsonResponse += "\"artistArtUri\": \"" + _mediaItem.artistArtUri + "\", ";
		jsonResponse += "\"duration\": \"" + _mediaItem.duration + "\", ";
		jsonResponse += "\"id\": \"" + _mediaItem.id + "\", ";
		jsonResponse += "\"raumfeldSection\": \"" + _mediaItem.raumfeldSection + "\", ";
		jsonResponse += "\"res\": \"" + _mediaItem.res + "\", ";
		jsonResponse += "\"type\": \"" + std::to_string(boost::uint8_t(_mediaItem.type)) + "\", ";
		jsonResponse += "\"albumArtUri\": \"" + _mediaItem.albumArtUri + "\" ";
		jsonResponse += "}";

		return jsonResponse;
	}

	ResponseHandlerResponseReturn JSONResponseHandler::ResponseZonePlaylist(std::string _zoneUDN, std::string _updateId, std::string _sessionId)
	{
		std::string playlistUpdateId = "";
		std::string jsonResponse = "";
		std::string	zoneListId = Utils::FormatUDN(_zoneUDN);
		bool hasItem = false;
		bool hasPlaylist = false;

		if (_zoneUDN.empty())	
			zoneListId = ALLZONEPLAYLISTSUPDATEID;

		// this is our long polling while loop
		playlistUpdateId = std::to_string(managerList.contentManager->GetLastUpdateIdForList(zoneListId));
		while (!_updateId.empty() && _updateId == playlistUpdateId)
		{
			this->Sleep(100);
			// last time zone config was changed is given on zone Manager!
			playlistUpdateId = std::to_string(managerList.contentManager->GetLastUpdateIdForList(zoneListId));

			// skip on abort!
			if (std::find(abortRequestIdList.begin(), abortRequestIdList.end(), _sessionId) != abortRequestIdList.end() || cancelRequests)
			{
				ResponseHandlerResponseReturn responseReturn;
				responseReturn.content = "aborted";
				return responseReturn;
			}
		}


		try
		{

			managerList.deviceManager->Lock();
			managerList.zoneManager->Lock();

			jsonResponse += "{";
			jsonResponse += "\"zonePlaylist\":[";

			for (auto zoneInfoMap = managerList.zoneManager->GetZoneInformationMap()->begin(); zoneInfoMap != managerList.zoneManager->GetZoneInformationMap()->end(); zoneInfoMap++)
			{
				ZoneInformation zoneInfo = zoneInfoMap->second;

				hasItem = false;

				if (_zoneUDN.empty() || _zoneUDN == zoneInfo.UDN)
				{


					zoneListId = Utils::FormatUDN(zoneInfo.UDN);

					if (hasPlaylist)
						jsonResponse += ",";
					

					hasPlaylist = true;

					managerList.contentManager->Lock();
					bool inCache = managerList.contentManager->IsListInCache(zoneListId);
					managerList.contentManager->UnLock();

					if (!inCache)
						managerList.contentManager->StartGetMediaItemListByZoneUDN(Utils::FormatUDN(zoneInfo.UDN), true);


					std::list<MediaItem> zonePlaylist = managerList.contentManager->EndGetMediaItemList(zoneListId);

					jsonResponse += "{ \"zoneUDN\":\"" + zoneInfo.UDN + "\", \"playlistItem\":[";

					
					for (auto mediaItem : zonePlaylist)
					{
						if (hasItem)
							jsonResponse += ", ";
						jsonResponse += this->CreateMediaItemObjectJSONString(mediaItem);
						hasItem = true;
					}
					

					// create each zone		
					jsonResponse += "]}";
				}

				
			}

			jsonResponse += "]";
			jsonResponse += "}";

			//if (!hasItem)
			//	jsonResponse = "";

		}
		catch (std::exception &ex)
		{
			managerList.deviceManager->UnLock();
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			throw std::runtime_error(ex.what());
		}
		catch (...)
		{
			managerList.deviceManager->UnLock();
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, "Unknown Error!", __FUNCTION__);
			throw std::runtime_error("Unknown Error!");
		}

		managerList.deviceManager->UnLock();
		managerList.zoneManager->UnLock();


		ResponseHandlerResponseReturn responseReturn;

		responseReturn.content = jsonResponse;
		if (!playlistUpdateId.empty())
			responseReturn.header.insert(std::make_pair("updateID", playlistUpdateId));

		return responseReturn;
	}

	ResponseHandlerResponseReturn JSONResponseHandler::ResponseContainerList(std::string _containerId, bool _useCache)
	{
		std::string jsonResponse;
		bool hasItem = false;
		bool inCache = false;

		try
		{

			managerList.deviceManager->Lock();
			managerList.zoneManager->Lock();

			jsonResponse += "{";
			jsonResponse += "\"containerList\":[";
			
				
			if (hasItem)
				jsonResponse += ",";
		

			if (_useCache)
			{
				managerList.contentManager->Lock();
				inCache = managerList.contentManager->IsListInCache(_containerId);
				managerList.contentManager->UnLock();
			}

			if (!inCache)
				managerList.contentManager->StartGetMediaItemListByContainerId(_containerId, "", _containerId, false, true);

			std::list<MediaItem> containerList = managerList.contentManager->EndGetMediaItemList(_containerId);					
			
			for (auto mediaItem : containerList)
			{
				if (hasItem)
					jsonResponse += ", ";
				jsonResponse += this->CreateMediaItemObjectJSONString(mediaItem);
				hasItem = true;
			}
				

			jsonResponse += "]";
			jsonResponse += "}";
;

		}
		catch (std::exception &ex)
		{
			managerList.deviceManager->UnLock();
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			throw std::runtime_error(ex.what());
		}
		catch (...)
		{
			managerList.deviceManager->UnLock();
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, "Unknown Error!", __FUNCTION__);
			throw std::runtime_error("Unknown Error!");
		}

		managerList.deviceManager->UnLock();
		managerList.zoneManager->UnLock();


		ResponseHandlerResponseReturn responseReturn;

		responseReturn.content = jsonResponse;	

		return responseReturn;
	}


	std::string JSONResponseHandler::CreateMediaRendererStateObjectJSONString(ZoneInformation _zoneInfo, RaumfeldVirtualMediaRenderer* _mediaRenderer)
	{
		MediaRendererState rendererState = _mediaRenderer->GetMediaRendererState();
		std::string jsonResponse;
		MediaItem mediaItem = _mediaRenderer->GetCurrentMediaItem();

		jsonResponse += "{";
		jsonResponse += "\"zoneUDN\": \"" + _zoneInfo.UDN + "\", ";
		jsonResponse += "\"zoneName\": \"" + _zoneInfo.name + "\", ";
		jsonResponse += "\"volume\": \"" + std::to_string(rendererState.volume) + "\", ";
		jsonResponse += "\"trackDuration\": \"" + rendererState.currentTrackDuration + "\", ";
		jsonResponse += "\"trackNr\": \"" + std::to_string(rendererState.currentTrack) + "\", ";
		jsonResponse += "\"trackCount\": \"" + std::to_string(rendererState.numberOfTracks) + "\", ";
		jsonResponse += "\"contentType\": \"" + rendererState.contentType + "\", ";
		jsonResponse += "\"bitrate\": \"" + std::to_string(rendererState.bitrate) + "\", ";
		jsonResponse += "\"transportState\": \"" + MediaRenderer::MediaRendererTransportStateToString(rendererState.transportState) + "\", ";
		jsonResponse += "\"muteState\": \"" + MediaRenderer::MediaRendererMuteStateToString(rendererState.muteState) + "\", ";		
		jsonResponse += "\"trackTitle\": \"" + Utils::EncodeValue(mediaItem.title) + "\", ";
		jsonResponse += "\"trackAlbum\": \"" + Utils::EncodeValue(mediaItem.album) + "\", ";
		jsonResponse += "\"trackArtist\": \"" + Utils::EncodeValue(mediaItem.artist) + "\", ";
		jsonResponse += "\"trackArtistArtUri\": \"" + mediaItem .artistArtUri+ "\", ";
		jsonResponse += "\"trackRaumfeldSection\": \"" + mediaItem.raumfeldSection + "\", ";
		jsonResponse += "\"trackRes\": \"" + mediaItem.res + "\", ";
		jsonResponse += "\"trackType\": \"" + std::to_string(boost::uint8_t(mediaItem.type)) + "\", ";
		jsonResponse += "\"trackAlbumArtUri\": \"" + mediaItem.albumArtUri + "\" ";
		jsonResponse += "}";

		/*		
		std::string aVTransportURI;
		std::string aVTransportURIMetaData;
		std::string currentTrackURI;
		std::string currentTrackMetaData;
		std::string containerId;	
		*/

		return jsonResponse;
	}
	

	ResponseHandlerResponseReturn JSONResponseHandler::ResponseMediaRendererState(std::string _updateId, std::string _sessionId)
	{
		std::string rendererStateUpdateId = "";	
		std::string jsonResponse = "";
		bool hasRenderer= false;


		// this is our long polling while loop
		rendererStateUpdateId = std::to_string(managerList.deviceManager->GetLastUpdateIdRendererState());
		while (!_updateId.empty() && _updateId == rendererStateUpdateId)
		{
			this->Sleep(100);
			// last time zone config was changed is given on zone Manager!
			rendererStateUpdateId = std::to_string(managerList.deviceManager->GetLastUpdateIdRendererState());

			// skip on abort!
			if (std::find(abortRequestIdList.begin(), abortRequestIdList.end(), _sessionId) != abortRequestIdList.end() || cancelRequests)
			{
				ResponseHandlerResponseReturn responseReturn;
				responseReturn.content = "aborted";
				return responseReturn;
			}
		}

		try
		{

			managerList.deviceManager->Lock();
			managerList.zoneManager->Lock();

			jsonResponse += "{ \"rendererState\":[" ;

			for (auto mapIt = managerList.zoneManager->GetZoneInformationMap()->begin(); mapIt != managerList.zoneManager->GetZoneInformationMap()->end(); mapIt++)
			{
				ZoneInformation zoneInfo = (ZoneInformation)mapIt->second;
				RaumfeldVirtualMediaRenderer *mediaRenderer = (RaumfeldVirtualMediaRenderer*)managerList.deviceManager->GetMediaRenderer(zoneInfo.UDN);
				if (mediaRenderer)
				{
					if (hasRenderer)
						jsonResponse += ", ";
					jsonResponse += this->CreateMediaRendererStateObjectJSONString(zoneInfo, mediaRenderer);
					hasRenderer = true;
				}
			}
			
			// create each zone		
			jsonResponse += "]}";

			if (!hasRenderer)
				jsonResponse = "";

		}
		catch (std::exception &ex)
		{
			managerList.deviceManager->UnLock();
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			throw std::runtime_error(ex.what());
		}
		catch (...)
		{
			managerList.deviceManager->UnLock();
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, "Unknown Error!", __FUNCTION__);
			throw std::runtime_error("Unknown Error!");
		}

		managerList.deviceManager->UnLock();
		managerList.zoneManager->UnLock();

		ResponseHandlerResponseReturn responseReturn;

		responseReturn.content = jsonResponse;
		if (!rendererStateUpdateId.empty())
			responseReturn.header.insert(std::make_pair("updateID", rendererStateUpdateId));

		return responseReturn;
	}


	ResponseHandlerResponseReturn JSONResponseHandler::ResponseZoneList(std::string _updateId, std::string _sessionId)
	{
		std::string zoneUpdateId = "";
		boost::unordered_map<std::string, ZoneInformation> *zoneInformationMap = nullptr;
		std::string jsonResponse = "", unassignedJson = "";
		bool hasZone = false;

		// this is our long polling while loop
		zoneUpdateId = std::to_string(managerList.zoneManager->GetLastUpdateId());
		while (!_updateId.empty() && _updateId == zoneUpdateId)
		{
			this->Sleep(100);
			// last time zone config was changed is given on zone Manager!
			zoneUpdateId = std::to_string(managerList.zoneManager->GetLastUpdateId());
			
			// skip on abort!
			if (std::find(abortRequestIdList.begin(), abortRequestIdList.end(), _sessionId) != abortRequestIdList.end() || cancelRequests)
			{
				ResponseHandlerResponseReturn responseReturn;
				responseReturn.content = "aborted";
				return responseReturn;
			}
		}

		try
		{
			
			managerList.zoneManager->Lock();
			zoneInformationMap = managerList.zoneManager->GetZoneInformationMap();

			jsonResponse += "{ \"zone\":[";

			for (auto it = zoneInformationMap->begin(); it != zoneInformationMap->end(); it++)
			{
				ZoneInformation zoneInfo = it->second;	

				if (hasZone)
					jsonResponse += ", ";
				jsonResponse += this->CreateZoneObjectJSONString(zoneInfo, true);				
				hasZone = true;
			}		

			// unassigned rooms!			
			unassignedJson = this->CreateUnassignedZoneObjectJSONString(true);
			if (!unassignedJson.empty())
			{
				if (hasZone)
					jsonResponse += ", ";
				jsonResponse += unassignedJson;
				hasZone = true;
			}					

			jsonResponse += "]}";	


			if (!hasZone)
				jsonResponse = "";

		}
		catch (std::exception &ex)
		{			
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			throw std::runtime_error(ex.what());
		}
		catch (...)
		{
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, "Unknown Error!", __FUNCTION__);
			throw std::runtime_error("Unknown Error!");
		}

		managerList.zoneManager->UnLock();

		ResponseHandlerResponseReturn responseReturn;

		responseReturn.content = jsonResponse;
		if (!zoneUpdateId.empty())
			responseReturn.header.insert(std::make_pair("updateID", zoneUpdateId));

		return responseReturn;
	}


	std::string JSONResponseHandler::CreateZoneObjectJSONString(ZoneInformation _zoneInformation, bool _addRoomInformation)
	{
		std::string jsonResponse, roomJson;
		bool hasRoom = false;

		if (_addRoomInformation)
		{
			for (auto it = _zoneInformation.roomsUDN.begin(); it != _zoneInformation.roomsUDN.end(); it++)
			{
				RoomInformation roomInfo = managerList.zoneManager->GetRoomInformation(*it);
				if (hasRoom)
					roomJson += ", ";
				roomJson += this->CreateRoomObjectJSONString(roomInfo);
				hasRoom = true;
			}
			
			roomJson = "[" + roomJson + "]";
			
		}

		jsonResponse += "{";
		jsonResponse += "\"zoneUDN\": \"" + _zoneInformation.UDN + "\", ";
		jsonResponse += "\"zoneName\": \"" + _zoneInformation.name + "\" ";	
		jsonResponse += ",\"rooms\": " + roomJson;
		jsonResponse += "}";

		return jsonResponse;
	}

	std::string JSONResponseHandler::CreateUnassignedZoneObjectJSONString(bool _addRoomInformation)
	{
		std::string jsonResponse, roomJson;
		bool hasRoom = false;

		if (_addRoomInformation)
		{
			for (auto it = managerList.zoneManager->GetRoomInformationMap()->begin(); it != managerList.zoneManager->GetRoomInformationMap()->end(); it++)
			{
				RoomInformation roomInfo = it->second;
				if (roomInfo.zoneUDN.empty())
				{
					if (hasRoom)
						roomJson += ", ";
					roomJson += this->CreateRoomObjectJSONString(roomInfo);
					hasRoom = true;					
				}
			}

			roomJson = "[" + roomJson + "]";
		
		}

		jsonResponse += "{";
		jsonResponse += "\"zoneUDN\": \" \", ";
		jsonResponse += "\"zoneName\": \"Unassigned rooms\" ";	
		jsonResponse += ",\"rooms\": " + roomJson;
		jsonResponse += "}";

		return jsonResponse;
	}


	std::string JSONResponseHandler::CreateRoomObjectJSONString(RoomInformation _roomInformation)
	{
		std::string jsonResponse, roomColorHTML = "#FFFFFF"; // TODO: @@@ Maybe int value???

		jsonResponse += "{";
		jsonResponse += "\"roomUDN\": \"" + _roomInformation.UDN + "\", ";
		jsonResponse += "\"roomName\": \"" + _roomInformation.name + "\", ";
		jsonResponse += "\"roomColor\": \"" + roomColorHTML + "\", ";
		jsonResponse += "\"zoneUDN\": \"" + _roomInformation.zoneUDN + "\" ";
		jsonResponse += "}";

		/*
		eg.:
		[
		{"roomUDN": "uid:45454y4545s4d4545d", "roomName": "Schlafzimmer", "roomColor": "#ff00ff", "zoneUDN": ""}
		{"roomUDN": "uid:45454y4545s4d4545d", "roomName": "Wohnzimmer", "roomColor": "#ff00ff", "zoneUDN": ""}
		{"roomUDN": "uid:45454y4545s4d4545d", "roomName": "Bad", "roomColor": "#ff00ff", "zoneUDN": "uid:45454y4545s4d4545d"}
		{"roomUDN": "uid:45454y4545s4d4545d", "roomName": "Kueche", "roomColor": "#ff00ff", "zoneUDN": "uid:45454y4545s4d4545d"}
		]
		*/

		return jsonResponse;
	}

	ResponseHandlerResponseReturn JSONResponseHandler::ResponseRoomList(std::string _updateId, std::string _sessionId )
	{
		std::string zoneUpdateId = "";
		boost::unordered_map<std::string, RoomInformation> *roomInformationMap = nullptr;
		std::string jsonResponse = "";
		bool hasRoom = false;

		// this is our long polling while loop
		zoneUpdateId = std::to_string(managerList.zoneManager->GetLastUpdateId());
		while (!_updateId.empty() && _updateId == zoneUpdateId)
		{
			this->Sleep(100);
			// last time zone config was changed is given on zone Manager!
			zoneUpdateId = std::to_string(managerList.zoneManager->GetLastUpdateId());

			// skip on abort!
			if (std::find(abortRequestIdList.begin(), abortRequestIdList.end(), _sessionId) != abortRequestIdList.end() || cancelRequests)
			{
				ResponseHandlerResponseReturn responseReturn;
				responseReturn.content = "aborted";
				return responseReturn;
			}
		}

		try
		{

			managerList.zoneManager->Lock();
			roomInformationMap = managerList.zoneManager->GetRoomInformationMap();

			jsonResponse += "[";

			for (auto it = roomInformationMap->begin(); it != roomInformationMap->end(); it++)
			{
				RoomInformation roomInfo = it->second;	

				if (hasRoom)
					jsonResponse += ", ";

				jsonResponse += this->CreateRoomObjectJSONString(roomInfo);			
				hasRoom = true;			
			}

			jsonResponse += "]";

			if (!hasRoom)
				jsonResponse = "";

		}
		catch (std::exception &ex)
		{
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			throw std::runtime_error(ex.what());
		}
		catch (...)
		{
			managerList.zoneManager->UnLock();
			this->Log(LogType::LOGERROR, "Unknown Error!", __FUNCTION__);
			throw std::runtime_error("Unknown Error!");
		}

		managerList.zoneManager->UnLock();

		ResponseHandlerResponseReturn responseReturn;

		responseReturn.content = jsonResponse;
		if (!zoneUpdateId.empty())
			responseReturn.header.insert(std::make_pair("updateID", zoneUpdateId));

		return responseReturn;
	}




	HTMLResponseHandler::HTMLResponseHandler() : ResponseHandler()
	{
	}

	HTMLResponseHandler::~HTMLResponseHandler()
	{
	}

	ResponseHandlerResponseReturn HTMLResponseHandler::CreateResponseForRequest(std::string _requestPath, std::unordered_map<std::string, std::string> _header, std::string _content)
	{
		std::vector<std::string> urlVector, filePathAndQueryVector;
		std::string actionAndQuery = "", urlQuery = "", file = "", updateId = "", filePathAndQuery= "";
		bool binaryResponse = false;

		// vector positions of "raumserver" and "webclient" are always present! (we insured that with the regex values on server start)		
		urlVector = Utils::ExplodeString(_requestPath, "/");
		if (urlVector.size() < 4)
		{
			this->Log(LogType::LOGERROR, "PATH For Request not correct!", __FUNCTION__);
			throw std::runtime_error("PATH For Request not correct! '" + _requestPath + "'");
		}

		//
		for (boost::uint16_t i = 4; i <= urlVector.size(); i++)
		{
			if (!filePathAndQuery.empty())
				filePathAndQuery += "/";
			filePathAndQuery += Utils::Unescape(urlVector[i-1]);
		}
	
		filePathAndQueryVector = Utils::ExplodeString(filePathAndQuery, "?");
		if (filePathAndQueryVector.size() > 1)
			urlQuery = filePathAndQueryVector[1];
		file = filePathAndQueryVector[0];

		if (!file.empty())
		{
			auto it = _header.find("Accept");
			if (it != _header.end())
			{
				std::string acceptStr = it->second;
				if (acceptStr.find("image") != std::string::npos)
				{
					binaryResponse = true;
				}
			}

			//A simple platform-independent file-or-directory check do not exist, but this works in most of the cases:
			if (file.find('.') == std::string::npos) {
				if (file[file.length() - 1] != '/')
					file += '/';
				file += "index.html";
			}

			ResponseHandlerResponseReturn responseReturn;
			responseReturn.binaryContent = this->ReadFileFromWebDirectory(raumkernSettings.RAUMKERNEL_WEBCLIENT_ROOTPATH + file, binaryResponse);
			responseReturn.content = std::string(responseReturn.binaryContent.begin(), responseReturn.binaryContent.end());
			responseReturn.isBinaryResponse = binaryResponse;
						
			return responseReturn;
		}
		else
		{
			throw std::runtime_error("Webclient file not defined!");
		}

	}

	std::vector<char> HTMLResponseHandler::ReadFileFromWebDirectory(std::string _fileName, bool _binary)
	{
		std::ifstream ifs;
			
		
		if (_binary)
			ifs.open(_fileName, std::ifstream::binary);
		else
			ifs.open(_fileName, std::ifstream::in);

		if (ifs) 
		{		

			ifs.rdbuf();

			std::vector<char> fileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		
			ifs.close();

			return fileContent;
		}
		else 
		{
			throw std::runtime_error("File '" + _fileName + "' not found");
		}
	}

	/*
	std::string HTMLResponseHandler::ReadFileFromWebDirectory(std::string _fileName, bool _binary)
	{
		std::ifstream ifs;


		ifs.open(_fileName, std::ifstream::in);

		if (ifs)
		{
			ifs.rdbuf();

			std::string fileContent((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());


			ifs.close();

			return fileContent;
		}
		else
		{
			throw std::runtime_error("File '" + _fileName + "' not found");
		}
		
	}*/

}