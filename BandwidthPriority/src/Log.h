#pragma once
#include <variant>
#include "Packet.h"
#include "Divert.h"

namespace BandwidthPriority
{
	enum LogLevel
	{
		Info, Debug, Warning, Error, Fatal
	};
	class Log
	{
	public:
		// Print level for the log functions. Everything including this level will be printed.
		// Set to Info for debug and to Error for release.
		static const LogLevel printLevel = LogLevel::Info;

		static void log(LogLevel level, std::string&& errorMessage);
		static void log(LogLevel level, std::wstring&& errorMessage);

		// Print every logged message with a LogLevel >= threshold.
		static void PrintLog(LogLevel threshold);

		// Print packet details for debug/visualization
		static void PrintHeaderDetails(const Packet& packet);
		static void PrintAddressDetails(const WINDIVERT_ADDRESS& address);
		static void PrintNetworkTuple(const NetworkTuple& tuple);
	/*private:
		struct LogMessage
		{
			LogMessage(LogLevel level, std::string&& errorMessage);
			LogMessage(LogLevel level, std::wstring&& errorMessage);

			std::variant<std::string, std::wstring> m_message;
			LogLevel level = LogLevel::Info;
			std::chrono::time_point<std::chrono::system_clock> timestamp;
		};*/

		// static std::vector<LogMessage> logs;
	};
}