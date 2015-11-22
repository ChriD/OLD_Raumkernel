#ifndef LOGGER_H_INCLUDED
#define LOGGER_H_INCLUDED


#include "os.h"

#include <string>
#include <iostream>
#include <fstream>
#include <vector>

#include <boost/function.hpp>
#include <boost/cstdint.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/signals2.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/thread/mutex.hpp>



namespace ApplicationLogging
{

	enum class LogType { LOGDEBUG = 0 , LOGINFO = 1, LOGWARNING = 2, LOGERROR = 3, LOGCRITICAL = 4 };
	enum class LogAdapter { LOGADP_NONE = 0, LOGADP_SIGNAL = 1, LOGADP_FILE = 2 };

	const std::string LOGFILEPATH = "application.log";

    struct LogData
    {
        LogType type;
        std::string log;
		std::string location;
        boost::posix_time::ptime time;
    };

    typedef boost::signals2::signal<void (LogData)> typeSignalLog;


    class Logger
    {
        public:
            Logger();
            virtual ~Logger();
			EXPORT void SubscribeSignalLog(const typeSignalLog::slot_type &_subscriber);
			EXPORT void Log(LogType _type, std::string _log, std::string _location = "", bool _forceLog = false);
			EXPORT void SetLogLevel(LogType _logType);
			EXPORT void AddLogAdapter(LogAdapter _logAdapter);
			EXPORT void ClearLogAdapters();
			EXPORT void ClearLogFile();
			EXPORT void SetLogFile(std::string _logFilePath);
			EXPORT static std::string GetTimeString(LogData _logData);
			EXPORT static std::string GetLogString(LogData _logData);
			EXPORT static std::string GetLogTypeString(LogType _logType);
			EXPORT static LogType GetLogTypeFromString(std::string _logTypeString);
			EXPORT static LogAdapter GetLogAdapterTypeFromString(std::string _logAdapterTypeString);
        protected:
			void DoLogAdapter(LogAdapter _logAdapterType, LogData _logData);
			void DoLogAdapterFile(LogData _logData);
			void DoLogAdapterSignal(LogData _logData);
			
			std::string logFilePath;

        private:
            typeSignalLog signalLog;
            LogType logLevel;
			boost::mutex mutexLog;
			std::vector<LogAdapter> assignedLogAdapters;

    };

}

#endif // LOGGER_H_INCLUDED
