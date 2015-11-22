#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED


#include "os.h"

#include <boost/cstdint.hpp>
#include <rapidxml/rapidxml.hpp>
#include <string>
#include <time.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/function_output_iterator.hpp>
#include <boost/bind.hpp>
#include <boost/unordered_map.hpp>

using namespace rapidxml;


namespace Raumkernel
{
	const std::string RAUMKERNEL_VERSION = "0.1.10.98";
	const std::string SETTINGS_FILEPATH = "settings.xml";
	const std::string REQUESTPATH = "Raumkern";
	const std::string ALLZONEPLAYLISTSUPDATEID = "AllZonePlsUpdateId";

	struct RAUMKERN_SETTINGS
	{
		std::string RAUMKERNEL_FRIENDLYNAME_CONFIGDEVICE;
		std::string RAUMKERNEL_FRIENDLYNAME_MEDIASERVER;
		std::string RAUMKERNEL_REQUESTPORT;
		std::string RAUMKERNEL_MEDIARENDERER_IDENTIFICATION;
		std::string RAUMKERNEL_MEDIASERVER_IDENTIFICATION;
		std::string RAUNKERNEL_DESCRIPTIONVIRTUALMEDIAPLAYER;
		std::string RAUMKERNEL_RAUMFELDMANUFACTURER;
		
		std::string RAUMKERNEL_SERVER_PORT;
		std::string RAUMKERNEL_SERVER_REQUESTPRESETFILEPATH;
		boost::uint16_t RAUMKERNEL_SERVER_TIMOUTREQUEST_SEC;
		boost::uint16_t RAUMKERNEL_SERVER_TIMOUTCONTENT_SEC;
		boost::uint16_t RAUMKERNEL_SERVER_FAILURERESTARTPOLLTIME;
		boost::uint16_t RAUMKERNEL_SERVER_MAXALLOWEDTHREADS;		
		bool RAUMKERNEL_SERVER_ENABLED;

		std::string RAUMKERNEL_WEBCLIENT_ROOTPATH;
	};

	// structure for roomInformation
	struct RoomInformation
	{
		std::string UDN;
		std::string zoneUDN;
		std::string name;
		std::string color;
		std::vector<std::string> rendererUDN;
	};


	// structure for zoneInformation
	struct ZoneInformation
	{
		std::string UDN;
		std::string name;
		std::vector<std::string> roomsUDN;
	};


	class Utils
	{
		public:
			Utils();
			virtual ~Utils();

			EXPORT static boost::uint32_t TimeStringToTimeMS(std::string _timeString);
			EXPORT static std::vector<std::string> ExplodeString(const std::string &inString, const std::string &separator);

			EXPORT static std::string EncodeUri(const std::string& _uri, bool _ecodeFromQuestionMark = false, std::string _additionalReserved = "");
			EXPORT static std::string EncodeUriPart(const std::string& _uri);
			EXPORT static std::string EncodeValue(const std::string& _value);
			EXPORT static std::string FormatUDN(std::string _udn);
			EXPORT static std::string Unescape(std::string _value);

			EXPORT static boost::uint32_t TrackIndexToTrackNumber(boost::int32_t _trackIndex);
			EXPORT static boost::int32_t TrackNumberToTrackIndex(boost::uint32_t _trackNumber);

			EXPORT static boost::unordered_map<std::string, std::string> ParseQueryString(std::string _queryString, bool _unescape = true, bool _tolowerKeyValues = true);

			EXPORT static boost::uint32_t GetRandomNumber();
			EXPORT static boost::int32_t TheAnswer();

			EXPORT static std::string CreateHTTPHeaderVars(boost::unordered_map<std::string, std::string> _vars);

	};

	extern RAUMKERN_SETTINGS raumkernSettings;
	
}


#endif 