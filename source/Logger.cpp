#include "Logger.h"

using namespace std;

namespace ApplicationLogging
{

	Logger::Logger()
	{
		this->logFilePath = LOGFILEPATH;
		this->logLevel = LogType::LOGDEBUG;
	}

	Logger::~Logger()
	{
	}


	// Logger::Log
	void Logger::Log(LogType _type, std::string _log, std::string _location, bool _forceLog)
	{
		boost::mutex::scoped_lock scoped_lock(mutexLog);

		LogData logData;

		// only do logging of log level which is set to be logged, all other infos will be thrown away
		if (_type < logLevel && !_forceLog)
			return;

		logData.type = _type;
		logData.log = _log;
		logData.location = _location;
		logData.time = boost::posix_time::microsec_clock::local_time();

		// run through attached adapters and do logging
 		for (std::vector<LogAdapter>::iterator it = assignedLogAdapters.begin(); it != assignedLogAdapters.end(); ++it)
		{						
			this->DoLogAdapter(*it, logData);
		}		
		
	}


	// Logger::DoLogAdapter
	void Logger::DoLogAdapter(LogAdapter _logAdapterType, LogData _logData)
	{
		switch (_logAdapterType)
		{
			case LogAdapter::LOGADP_SIGNAL:
				this->DoLogAdapterSignal(_logData);
				break;
			case LogAdapter::LOGADP_FILE:
				this->DoLogAdapterFile(_logData);
				break;
			default:
				break;
		}
	}

	// Logger::DoLogAdapterFile
	void Logger::DoLogAdapterFile(LogData _logData)
	{		
		ofstream myfile;

		myfile.open(logFilePath, fstream::out | fstream::app);
		if (myfile.is_open())
		{
			myfile << Logger::GetLogString(_logData) + "\n";
			myfile.close();
		}

	}


	// Logger::DoLogAdapterSignal
	void Logger::DoLogAdapterSignal(LogData _logData)
	{
		signalLog(_logData);
	}


	// Logger::SubscribeSignalLog
	void Logger::SubscribeSignalLog(const typeSignalLog::slot_type &_subscriber)
	{
		signalLog.connect(_subscriber);
	}


	// Logger::SetLogLevel
	void Logger::SetLogLevel(LogType _logType)
	{
		logLevel = _logType;
	}


	// Logger::GetTimeString
	// creates a std::string with the current time from a logdata struct
	std::string Logger::GetTimeString(LogData _logData)
	{
		std::stringstream stream;
		boost::posix_time::time_facet* facet = new boost::posix_time::time_facet();
		facet->format("%H:%M:%s");
		stream.imbue(std::locale(std::locale::classic(), facet));
		stream << _logData.time;
		return stream.str();
	}

	
	// Logger::GetLogString
	// returns a  prebuilt log string to use for any type of log output
	std::string Logger::GetLogString(LogData _logData)
	{
		if (_logData.type == LogType::LOGDEBUG)
			return GetTimeString(_logData) + " " + GetLogTypeString(_logData.type) + "\t" + _logData.log + " (" + _logData.location + ")";
		return GetTimeString(_logData) + " " + GetLogTypeString(_logData.type) + "\t" + _logData.log;
	}


	//Logger::GetLogTypeString
	// converts the logType enum to a 'readable' string value.
	// there might be better options to do this but for now it will do the work
	std::string Logger::GetLogTypeString(LogType _logType)
	{
		switch (_logType)
		{
		case LogType::LOGINFO:
			return "INFO";
		case LogType::LOGDEBUG:
			return "DEBUG";
		case LogType::LOGERROR:
			return "ERROR";
		case LogType::LOGWARNING:
			return "WARNING";
		case LogType::LOGCRITICAL:
			return "CRITIC";
		default:
			break;
		}
		return "UNKNO";
	}


	// Logger::GetLogTypeFromString
	// converts a logType string to an enum value
	// there might be better options to do this but for now it will do the work
	LogType Logger::GetLogTypeFromString(std::string _logTypeString)
	{
		if (_logTypeString == "INFO")
			return LogType::LOGINFO;
		if (_logTypeString == "DEBUG")
			return LogType::LOGDEBUG;
		if (_logTypeString == "WARNING" || _logTypeString == "WARN")
			return LogType::LOGWARNING;
		if (_logTypeString == "ERROR")
			return LogType::LOGERROR;
		if (_logTypeString == "CRITICAL" || _logTypeString == "CRITIC")
			return LogType::LOGERROR;
		return LogType::LOGDEBUG;
	}


	// Logger::GetLogAdapterTypeFromString
	// converts a logAdapterType string to an enum value
	LogAdapter Logger::GetLogAdapterTypeFromString(std::string _logAdapterTypeString)
	{
		if (_logAdapterTypeString == "SIGNAL" || _logAdapterTypeString == "SCREEN")
			return LogAdapter::LOGADP_SIGNAL;
		if (_logAdapterTypeString == "FILE")
			return LogAdapter::LOGADP_FILE;
		return LogAdapter::LOGADP_NONE;
	}


	// Logger::AddLogAdapter
	void Logger::AddLogAdapter(LogAdapter _logAdapter)
	{
		assignedLogAdapters.push_back(_logAdapter);
	}


	// Logger::ClearLogAdapters
	void Logger::ClearLogAdapters()
	{
		assignedLogAdapters.clear();
	}


	// Logger::ClearLogFile
	void Logger::ClearLogFile()
	{		
		remove(logFilePath.c_str());
	}


	// Logger::SetLogFile
	void Logger::SetLogFile(std::string _logFilePath)
	{
		logFilePath = _logFilePath;
	}


}