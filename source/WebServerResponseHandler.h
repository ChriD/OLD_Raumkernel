#ifndef WEBSERVERREQUESTHANDLER_INCLUDED
#define WEBSERVERREQUESTHANDLER_INCLUDED


#include "os.h"
#include "RaumkernObject.h"
#include "MediaItem.h"
#include "Manager.h"
#include "Renderer.h"

#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>

#include <rapidxml/rapidxml.hpp>

using namespace ApplicationLogging;
using namespace rapidxml;


namespace Raumkernel
{

	const std::string ACTION_JSON_GETROOMLIST = "getroomlist";
	const std::string ACTION_JSON_GETZONELIST = "getzonelist";
	const std::string ACTION_JSON_GETRENDERERSTATE = "getrendererstate";
	const std::string ACTION_JSON_GETZONEPLAYLIST = "getzoneplaylist";
	const std::string ACTION_JSON_GETCONTAINERLIST = "getcontainerlist";
	const std::string ACTION_ABORTREQUESTS = "abortrequests";

	
	struct ResponseHandlerResponseReturn
	{
		std::string content;
		std::vector<char> binaryContent;
		bool isBinaryResponse;
		boost::unordered_map<std::string, std::string> header;
	};


	class ResponseHandler : public RaumkernObject
	{
		public:
			ResponseHandler();
			virtual ~ResponseHandler();
			void SetCancelRequets(bool _cancel);

		protected:
			std::string GetQueryValue(std::string _uri, std::string _key);
			bool cancelRequests;
		
		private:		
	};



	class HTMLResponseHandler : public ResponseHandler
	{
		public:
			HTMLResponseHandler();
			virtual ~HTMLResponseHandler();

			ResponseHandlerResponseReturn CreateResponseForRequest(std::string _requestPath, std::unordered_map<std::string, std::string> _header, std::string _content);

		protected:
			std::vector<char> ReadFileFromWebDirectory(std::string _fileName, bool _binary = false);	

		private:
	};



	class JSONResponseHandler : public ResponseHandler
	{
		public:
			JSONResponseHandler();
			virtual ~JSONResponseHandler();

			ResponseHandlerResponseReturn CreateResponseForRequest(std::string _requestPath, std::unordered_map<std::string, std::string> _header, std::string _content);

		protected:		
			ResponseHandlerResponseReturn ResponseMediaRendererState(std::string _updateId = "", std::string _sessionId = "");
			ResponseHandlerResponseReturn ResponseZonePlaylist(std::string _zoneUDN, std::string _updateId = "", std::string _sessionId  = "");
			ResponseHandlerResponseReturn ResponseContainerList(std::string _containerId, bool _useCache = false);
			ResponseHandlerResponseReturn ResponseZoneList(std::string _updateId, std::string _sessionId = "");
			ResponseHandlerResponseReturn ResponseRoomList(std::string _updateId, std::string _sessionId = "");
			ResponseHandlerResponseReturn AbortRequests(std::string _id);

			std::list<std::string> abortRequestIdList;
			boost::mutex abortRequestListMutex;

		private:
			std::string CreateZoneObjectJSONString(ZoneInformation _zoneInformation, bool _addRoomInformation = true);
			std::string CreateUnassignedZoneObjectJSONString(bool _addRoomInformation = true);
			std::string CreateRoomObjectJSONString(RoomInformation _zoneInformation);
			std::string CreateMediaRendererStateObjectJSONString(ZoneInformation _zoneInfo, RaumfeldVirtualMediaRenderer* _mediaRenderer);
			std::string CreateMediaItemObjectJSONString(MediaItem _mediaItem);

			
	};




}

#endif // WEBSERVERREQUESTHANDLER_INCLUDED
