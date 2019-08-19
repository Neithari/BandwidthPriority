#pragma once
#include <variant>

namespace BandwidthPriority
{
	enum LogLevel
	{
		Info, Warning, Error, Fatal
	};
	class Log
	{
	public:
		static void log(LogLevel level, std::string&& errorMessage);
		static void log(LogLevel level, std::wstring&& errorMessage);

		// Print every logged message with a LogLevel >= threshold.
		static void PrintLog(LogLevel threshold);
	private:
		struct LogMessage
		{
			LogMessage(LogLevel level, std::string&& errorMessage);
			LogMessage(LogLevel level, std::wstring&& errorMessage);

			std::variant<std::string, std::wstring> m_message;
			LogLevel level = LogLevel::Info;
			std::chrono::time_point<std::chrono::system_clock> timestamp;
		};

		static std::vector<LogMessage> logs;
	};
}