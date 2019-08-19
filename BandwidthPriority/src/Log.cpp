#include "pch.h"
#include "Log.h"

void BandwidthPriority::Log::log(LogLevel level, std::string&& message)
{
	logs.emplace_back(level, std::move(message));
}

void BandwidthPriority::Log::log(LogLevel level, std::wstring&& message)
{
	logs.emplace_back(level, std::move(message));
}

void BandwidthPriority::Log::PrintLog(LogLevel threshold)
{
	for (auto& l : logs)
	{
		if (l.level >= threshold)
		{
			switch (l.level)
			{
			case BandwidthPriority::Info:
				std::cout << "Info: ";
				break;
			case BandwidthPriority::Warning:
				std::cout << "Warning: ";
				break;
			case BandwidthPriority::Error:
				std::cout << "Error: ";
				break;
			case BandwidthPriority::Fatal:
				std::cout << "Fatal: ";
				break;
			default:
				break;
			}

			if (std::holds_alternative<std::wstring>(l.m_message))
			{
				std::wcout << std::get<std::wstring>(l.m_message) << std::endl;
			}
			else
			{
				std::cout << std::get<std::string>(l.m_message) << std::endl;
			}
		}
	}
}

BandwidthPriority::Log::LogMessage::LogMessage(LogLevel level, std::string&& errorMessage)
	:
	level(level),
	timestamp(std::chrono::system_clock::now()),
	m_message(errorMessage)
{
}

BandwidthPriority::Log::LogMessage::LogMessage(LogLevel level, std::wstring&& errorMessage)
	:
	level(level),
	timestamp(std::chrono::system_clock::now()),
	m_message(errorMessage)
{
}
