#include "Global.h"
#include <boost/locale.hpp>
#include <locale>
#include <math.h>
#include "urlcpp/Url.h"
#include <random>

using namespace boost::posix_time;

namespace Raumkernel
{

	RAUMKERN_SETTINGS raumkernSettings;

	Utils::Utils()
	{
	}

	Utils::~Utils()
	{

	}

	std::string Utils::CreateHTTPHeaderVars(boost::unordered_map<std::string, std::string> _vars)
	{
		std::string	headerString;
		for (auto it = _vars.begin(); it != _vars.end(); it++)
		{
			if (!it->first.empty() && !it->second.empty())
				headerString += it->first + ": " + it->second + "\r\n";
		}
		return headerString;
	}

	boost::int32_t Utils::TheAnswer()
	{
		return 42;
	}


	boost::uint32_t Utils::GetRandomNumber()
	{
		std::mt19937  rng;
		//std::minstd_rand rng;
		boost::uint32_t seed = time(NULL);
		rng.seed(seed);

		//std::random_device rd;     // only used once to initialise engine
		//std::mt19937 rng(rd);      // random-number engine used	

		std::uniform_int_distribution<uint32_t> uint_dist(1000000, 9999999);
		return uint_dist(rng);
		//return (boost::uint32_t)rand();
	}


	std::string Utils::FormatUDN(std::string _udn)
	{
		if (_udn.find("uuid:") != std::string::npos)
			return _udn;
		return "uuid:" + _udn;
	}


	boost::uint32_t Utils::TimeStringToTimeMS(std::string _timeString)
	{
		boost::uint32_t timeInMs;

		if (_timeString.empty())
			return 0;

		ptime t(time_from_string("1970-01-01 " + _timeString));
		ptime start(boost::gregorian::date(1970, 1, 1));
		time_duration dur = t - start;
		time_t epoch = dur.total_seconds();
		std::tm *timeStruct = gmtime(&epoch);

		timeInMs = ((timeStruct->tm_hour * 60 * 60) + (timeStruct->tm_min * 60) + (timeStruct->tm_sec)) * 1000;

		return timeInMs;

	}


	std::vector<std::string> Utils::ExplodeString(const std::string &inString,
		const std::string &separator)
	{
		std::vector<std::string> returnVector;
		std::string::size_type start = 0;
		std::string::size_type end = 0;

		while ((end = inString.find(separator, start)) != std::string::npos)
		{
			returnVector.push_back(inString.substr(start, end - start));
			start = end + separator.size();
		}

		returnVector.push_back(inString.substr(start));

		return returnVector;
	}


	std::string Utils::EncodeValue(const std::string& _value)
	{
		return EncodeUri(_value, false, "/");
	}

	std::string Utils::EncodeUriPart(const std::string& _uri)
	{
		return EncodeUri(_uri, false, "");
	}

	std::string Utils::Unescape(std::string _value)
	{
		return url::Url::unescape(_value);
	}


	std::string Utils::EncodeUri(const std::string& _uri, bool _ecodeFromQuestionMark, std::string _additionalReserved)
	{
		boost::uint32_t startEncodePos = 0;

		if (_ecodeFromQuestionMark)
			startEncodePos = _uri.find("?");

		//RFC 3986 section 2.3 Unreserved Characters (January 2005)
		const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~" + _additionalReserved;

		std::string escaped = "";
		for (size_t i = startEncodePos; i<_uri.length(); i++)
		{
			if (unreserved.find_first_of(_uri[i]) != std::string::npos)
			{
				escaped.push_back(_uri[i]);
			}
			else
			{
				boost::uint16_t charValue = (boost::uint16_t)(unsigned char)_uri[i];
				if (charValue > 127)
				{
					std::ostringstream converted;
					boost::uint8_t level = (boost::uint8_t)floor((charValue - 128) / 64);
					converted << '%' << std::hex << std::uppercase << 194 + level;
					converted << '%' << std::hex << std::uppercase << (charValue)-64;
					escaped.append(converted.str());
				}
				else
				{
					std::ostringstream converted;
					converted << '%' << std::hex << std::uppercase << (boost::int32_t)(unsigned char)_uri[i];
					escaped.append(converted.str());
				}
			}
		}
		return escaped;
	}

	boost::uint32_t Utils::TrackIndexToTrackNumber(boost::int32_t _trackIndex)
	{
		return _trackIndex + 1;
	}

	boost::int32_t Utils::TrackNumberToTrackIndex(boost::uint32_t _trackNumber)
	{
		return _trackNumber - 1;
	}




	boost::unordered_map<std::string, std::string> Utils::ParseQueryString(std::string _queryString, bool _unescape, bool _tolowerKeyValues)
	{
		boost::unordered_map<std::string, std::string> queryValuesMap;
		std::vector<std::string> idValueParts;

		idValueParts = ExplodeString(_queryString, "&");
		for (auto &idValuePart : idValueParts)
		{
			std::vector<std::string> valueAndPart;
			valueAndPart = ExplodeString(idValuePart, "=");
			if (valueAndPart.size() < 2)
			{
				if (_unescape)
					valueAndPart[0] = Utils::Unescape(valueAndPart[0]);
				if (_tolowerKeyValues)
					boost::algorithm::to_lower(valueAndPart[0]);
				// we may have only an identifier with no value
				// example: '?sid=AAA&a&b&c'
				queryValuesMap.insert(std::make_pair(valueAndPart[0], ""));
			}
			else
			{
				// example: '?sid=AAA&a=1&b=2&c=3'
				if (_unescape)
				{
					valueAndPart[0] = Utils::Unescape(valueAndPart[0]);
					valueAndPart[1] = Utils::Unescape(valueAndPart[1]);
				}
				if (_tolowerKeyValues)
				{
					boost::algorithm::to_lower(valueAndPart[0]);
				}
				queryValuesMap.insert(std::make_pair(valueAndPart[0], valueAndPart[1]));
			}
		}

		return queryValuesMap;
	}
	
}