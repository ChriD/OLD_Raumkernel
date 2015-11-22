#include "Manager.h"
#include "webserver/client_http.hpp"

namespace Raumkernel
{
		Manager::Manager() : RaumkernObject()
		{			
		}

		Manager::~Manager()
		{

		}		



		HTTPRequestManager::HTTPRequestManager() : Manager()
		{
			requestUrl = "";
			curRequestSocket = nullptr;
		}

		HTTPRequestManager::~HTTPRequestManager()
		{
		}

		// HTTPRequestManager::SetRequestUrl
		void HTTPRequestManager::SetRequestUrl(std::string _requestUrl)
		{
			this->requestUrl = _requestUrl;
		}


		// HTTPRequestManager::Request
		/*
		client::response HTTPRequestManager::Request(std::string _request, boost::unordered_map<std::string, std::string> *_headerVars, boost::unordered_map<std::string, std::string> *_postVars, bool _useHostPrefix, boost::uint16_t _cycleRounds)
		{			
			std::string		longPollingUpdateId;
			std::string		requestUrlAction, urlPostVars;
			boost::uint16_t cycles = 0;

			if (_useHostPrefix)
				requestUrlAction = requestUrl + _request;
			else
				requestUrlAction = _request;		

			this->Log(LogType::LOGDEBUG, "Doing request: '" + requestUrlAction + "'", __FUNCTION__);

			// if we got some '_postVars' we use this. For this we have to add the post values
			if (_postVars)
			{
				for (auto &mapItem : *_postVars)
				{
					if (!urlPostVars.empty())
						urlPostVars += "&";
					urlPostVars += Utils::EncodeUriPart(mapItem.first) + "=" + Utils::EncodeUriPart(mapItem.second);
				}
				requestUrlAction += "?" + urlPostVars;
			}

			boost::shared_ptr<boost::asio::io_service> io_service = boost::make_shared<boost::asio::io_service>();
			//http::client httpClient(client::options().io_service(io_service)); // TODO :@@@
			http::client httpClient;


			client::request httpRequest(requestUrlAction);
			
			

			// if we got some '_headerVars' we use this. For this we have to add a header with the value 
			if (_headerVars)
			{
				for (auto &mapItem : *_headerVars)
				{
					httpRequest << header(mapItem.first, mapItem.second);
				}
			}



			// get response of request and wait till its finished 
			// if we need async, then start this request in a thread	
			// ATTENTION: Because we are using cpp-netlib, the 'client::response httpResponse = httpClient.get(httpRequest);' will kill our interrupts if this method is called  in thread
			// we have to live with this issue for nor			
			client::response httpResponse = httpClient.get(httpRequest);
			try
			{
				while (!ready(httpResponse))
				{
					this->Sleep(50);
					boost::this_thread::interruption_point();
					cycles++;
					if (_cycleRounds > 0 && cycles > _cycleRounds)
					{
						throw std::runtime_error("Cycle rounds exceeded!");
					}
				}

			}
			catch (boost::thread_interrupted const&)
			{
				//
				//io_service->stop();			
			}


			this->Log(LogType::LOGDEBUG, "Request: '" + requestUrlAction + "' done...", __FUNCTION__);

			return httpResponse;
		}*/

		// HTTPRequestManager::Request		
		std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response>  HTTPRequestManager::Request(std::string _request, boost::unordered_map<std::string, std::string> *_headerVars, boost::unordered_map<std::string, std::string> *_postVars, bool _useHostPrefix, boost::uint16_t _cycleRounds)
		{
			std::string		longPollingUpdateId, ipAndPort, pathAndQuery;
			std::string		requestUrlAction, urlPostVars;
			boost::uint16_t cycles = 0;
			std::map<std::string, std::string> headerVars;

			if (_useHostPrefix)
				requestUrlAction = requestUrl + _request;
			else
				requestUrlAction = _request;
		
			boost::network::uri::uri instance(requestUrlAction);
			ipAndPort = instance.host();
			ipAndPort += !instance.port().empty() ? ":" + instance.port() : "";
			pathAndQuery = instance.path();
			//pathAndQuery += !instance.query().empty() ? "?" + instance.query() : "";

			this->Log(LogType::LOGDEBUG, "Doing request: '" + requestUrlAction + "'", __FUNCTION__);

			// if we got some '_postVars' we use this. For this we have to add the post values
			if (_postVars)
			{
				for (auto &mapItem : *_postVars)
				{
					if (!urlPostVars.empty())
						urlPostVars += "&";
					urlPostVars += Utils::EncodeUriPart(mapItem.first) + "=" + Utils::EncodeUriPart(mapItem.second);
				}
				pathAndQuery += "?" + urlPostVars;
			}

			// if we got some '_headerVars' we use this
			if (_headerVars)
			{
				for (auto &mapItem : *_headerVars)
				{
					headerVars.insert(std::make_pair(mapItem.first, mapItem.second));
				}
			}
		
			
			SimpleWeb::Client<SimpleWeb::HTTP> client(ipAndPort);
			this->Sleep(500);
			
			
			try
			{
			
				curRequestSocket = client.socket.get();
				auto httpResponse = client.request("GET", pathAndQuery, headerVars);						
				this->Log(LogType::LOGDEBUG, "Request: '" + requestUrlAction + "' done...", __FUNCTION__);
				curRequestSocket = nullptr; // only valid while getting request!
				return httpResponse;
			
			}
			catch (boost::thread_interrupted const&)
			{			
				this->Log(LogType::LOGDEBUG, "Request: '" + requestUrlAction + "' interrupted...", __FUNCTION__);
			}	
			catch (std::exception &ex)
			{
				this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
			}
			catch (...)
			{
				this->Log(LogType::LOGERROR, "Unknown error!", __FUNCTION__);
			}
			
			curRequestSocket = nullptr;
			return nullptr;
		}

		// HTTPRequestManager::GetLongPollingIdFromHeader
		/*
		std::string HTTPRequestManager::GetLongPollingIdFromResponse(client::response _httpResponse)
		{
			std::string longPollingUpdateId;		

			typedef http::basic_client<http::tags::http_default_8bit_tcp_resolve, 1, 0> http_client;
			typedef boost::network::headers_range<http_client::response>::type response_headers;
			typedef std::pair<std::string, std::string> header_type;
						
			// get 'updateId' from response header for 'long polling' option			
			response_headers headers_2 = headers(_httpResponse)["updateId"];
			BOOST_FOREACH(header_type const & header, headers_2)
			{
				longPollingUpdateId = header.second.c_str();
			}

			this->Log(LogType::LOGDEBUG, "Read long polling id from response (" + longPollingUpdateId + ")", __FUNCTION__);

			return longPollingUpdateId;
		}*/
		// HTTPRequestManager::GetLongPollingIdFromHeader
		std::string HTTPRequestManager::GetLongPollingIdFromResponse(std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> _httpResponse)
		{
			std::string longPollingUpdateId = "";
			std::unordered_map<std::string, std::string>::iterator it;
			std::unordered_map<std::string, std::string> headerValues;

			headerValues = _httpResponse->header;
			for (auto headerMapValue : headerValues)
			{
				std::string key = headerMapValue.first;
				boost::algorithm::to_lower(key);
				if (key == "updateid")
				{
					longPollingUpdateId = headerMapValue.second;
					break;
				}
			}
			
			this->Log(LogType::LOGDEBUG, "Read long polling id from response (" + longPollingUpdateId + ")", __FUNCTION__);

			return longPollingUpdateId;
		}




		// AliveCheckManager::AliveCheckManager
		AliveCheckManager::AliveCheckManager() : HTTPRequestManager()
		{
			aliveCheckTimeMS = 5000;
			isAlive = false;
		}

		// AliveCheckManager::AliveCheckManager
		AliveCheckManager::~AliveCheckManager()
		{
			this->InterruptAliveCheckThread();
		}
		
		// AliveCheckManager::AliveCheckManager
		void AliveCheckManager::SubscribeSystemAliveStateChanged(const typeSignalSystemAliveStateChanged::slot_type &_subscriber)
		{
			signalSystemAliveStateChanged.connect(_subscriber);
		}

		// AliveCheckManager::StartAliveCheck
		void AliveCheckManager::StartAliveCheck()
		{
			this->Log(LogType::LOGDEBUG, "Starting worker thread for check alive pings...", __FUNCTION__);	
			this->InterruptAliveCheckThread();
			checkAliveThread = boost::thread(&AliveCheckManager::AliveCheckThread, this);
		}


		// AliveCheckManager::AliveCheckThread
		void AliveCheckManager::AliveCheckThread()
		{
			std::shared_ptr<SimpleWeb::ClientBase<SimpleWeb::HTTP>::Response> httpResponse;
			std::string responseBody;
			std::ostringstream ss;

			while (true)
			{
				this->Log(LogType::LOGDEBUG, "PING! Raumfeld system! Are you alive?", __FUNCTION__);

				try
				{
					httpResponse = this->Request("Ping", nullptr, nullptr);
					if (httpResponse)
					{
						ss << httpResponse->content.rdbuf();
						responseBody = ss.str();
						if (isAlive == false)
							signalSystemAliveStateChanged(true);
						isAlive = true;
						this->Log(LogType::LOGDEBUG, "PONG! Yep Sir! I am Alive!", __FUNCTION__);
					}
					else
					{
						if (isAlive == true)
							signalSystemAliveStateChanged(false);
						isAlive = false;
						this->Log(LogType::LOGERROR, "Raumfeld System does not respond!", __FUNCTION__);
					}

					boost::this_thread::interruption_point();
					this->Sleep(aliveCheckTimeMS);
					boost::this_thread::interruption_point();
					
				}
				catch (std::exception &ex)
				{
					if (isAlive == true)
						signalSystemAliveStateChanged(false);
					isAlive = false;
					this->Log(LogType::LOGERROR, "Raumfeld System does not respond!", __FUNCTION__);
					this->Log(LogType::LOGERROR, ex.what(), __FUNCTION__);
					this->Sleep(aliveCheckTimeMS);
				}	
				catch (boost::thread_interrupted const&)
				{
					// interruption
					this->Log(LogType::LOGDEBUG, "PING-PONG interrupted", __FUNCTION__);
					break;
				}
			}
		}

		// AliveCheckManager::InterruptAliveCheckThread
		void AliveCheckManager::InterruptAliveCheckThread()
		{
			this->Log(LogType::LOGDEBUG, "Stopping alive thread manager", __FUNCTION__);

			if (checkAliveThread.joinable())
			{
				checkAliveThread.interrupt();
				checkAliveThread.join();
			}		

			this->Log(LogType::LOGDEBUG, "Alive thread manager stopped!", __FUNCTION__);
		}

		void AliveCheckManager::SetAliveCheckTime(boost::uint32_t _aliveCheckTimeMS)
		{
			aliveCheckTimeMS = _aliveCheckTimeMS;
		}

		bool AliveCheckManager::IsAlive()
		{
			return isAlive;
		}

}
