#ifndef MANAGER_H_INCLUDED
#define MANAGER_H_INCLUDED


#include "os.h"
#include <string>
#include <boost/cstdint.hpp>
#include <boost/signals2.hpp>
#include <boost/function.hpp>
#include <boost/network/uri.hpp>
#include <boost/thread.hpp>
#include <boost/fusion/include/map.hpp>
#include <boost/asio/io_service.hpp>

#include "Logger.h"
#include "Global.h"
#include "RaumkernObject.h"
#include "webserver/client_http.hpp"

using namespace ApplicationLogging;

//using namespace boost::network;
//using namespace boost::network::http;


namespace Raumkernel
{

		const std::string LONGPOLLINGREQUESTQUIT = "-1";

		typedef boost::signals2::signal<void(bool)> typeSignalSystemAliveStateChanged;

		class Manager : public RaumkernObject
		{
			public:
				Manager();
				virtual ~Manager();	
				
			protected:		

			private:
	
		};


		class HTTPRequestManager : public Manager
		{
			public:
				HTTPRequestManager();
				virtual ~HTTPRequestManager();
				void SetRequestUrl(std::string _requestUrl);
				//client::response Request(std::string _request, boost::unordered_map<std::string, std::string> *_headerVars = nullptr, boost::unordered_map<std::string, std::string> *_postVars = nullptr, bool _useHostPrefix = true, boost::uint16_t _cycleRounds = 0);
				std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> Request(std::string _request, boost::unordered_map<std::string, std::string> *_headerVars = nullptr, boost::unordered_map<std::string, std::string> *_postVars = nullptr, bool _useHostPrefix = true, boost::uint16_t _cycleRounds = 0);
				std::string GetLongPollingIdFromResponse(std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> _httpResponse);
	
			protected:
				std::string	requestUrl;			
				SimpleWeb::HTTP *curRequestSocket;

			private:									
		};


		class AliveCheckManager : public HTTPRequestManager
		{
			public:
				AliveCheckManager();
				virtual ~AliveCheckManager();
				
				EXPORT void SubscribeSystemAliveStateChanged(const typeSignalSystemAliveStateChanged::slot_type &_subscriber);
				EXPORT void StartAliveCheck();
				EXPORT void SetAliveCheckTime(boost::uint32_t _aliveCheckTimeMS);
				EXPORT bool IsAlive();

			protected:	
				void AliveCheckThread();
				void InterruptAliveCheckThread();

				boost::uint32_t aliveCheckTimeMS;

			private:
				boost::thread							checkAliveThread;
				typeSignalSystemAliveStateChanged		signalSystemAliveStateChanged;
				bool									isAlive;
		};


}

#endif // MANAGER_H_INCLUDED
