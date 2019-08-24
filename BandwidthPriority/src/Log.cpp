#include "pch.h"
#include "Log.h"

void BandwidthPriority::Log::log(LogLevel level, std::string&& message)
{
	// logs.emplace_back(level, std::move(message));
	switch (level)
	{
	case BandwidthPriority::Info:
		std::cout << "Info:\t ";
		break;
	case BandwidthPriority::Debug:
		std::cout << "Debug:\t ";
		break;
	case BandwidthPriority::Warning:
		std::cout << "Warning: ";
		break;
	case BandwidthPriority::Error:
		std::cout << "Error:\t ";
		break;
	case BandwidthPriority::Fatal:
		std::cout << "Fatal:\t ";
		break;
	default:
		break;
	}

	std::cout << message << std::endl;
}

void BandwidthPriority::Log::log(LogLevel level, std::wstring&& message)
{
	// logs.emplace_back(level, std::move(message));
	switch (level)
	{
	case BandwidthPriority::Info:
		std::wcout << L"Info:\t ";
		break;
	case BandwidthPriority::Debug:
		std::wcout << L"Debug:\t ";
		break;
	case BandwidthPriority::Warning:
		std::wcout << L"Warning: ";
		break;
	case BandwidthPriority::Error:
		std::wcout << L"Error:\t ";
		break;
	case BandwidthPriority::Fatal:
		std::wcout << L"Fatal:\t ";
		break;
	default:
		break;
	}

	std::wcout << message << std::endl;
}

void BandwidthPriority::Log::PrintLog(LogLevel threshold)
{
	/*for (auto& l : logs)
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
	}*/
}

// Print packet details for debug/visualization

inline void BandwidthPriority::Log::PrintHeaderDetails(const Packet& packet)
{
	Header header(packet);

	std::cout <<"SourceAddr:\t"	<< header.GetSource()			<< "\n" <<
				"SourcePort:\t"	<< header.GetSourcePort()		<< "\n" <<
				"DestAddr:\t"	<< header.GetDestination()		<< "\n" <<
				"DestPort:\t"	<< header.GetDestinationPort()	<< "\n" <<
				"Protocol:\t"	<< (int)header.protocol			<< "\n" << std::endl;
}

inline void BandwidthPriority::Log::PrintAddressDetails(const WINDIVERT_ADDRESS& address)
{
	if (address.Layer == WINDIVERT_LAYER_FLOW)
	{
		const UINT32* src, * dest;
		UINT16 srcPort, destPort;
		if (address.Outbound == 1)
		{
			src = address.Flow.LocalAddr;
			dest = address.Flow.RemoteAddr;

			srcPort = address.Flow.LocalPort;
			destPort = address.Flow.RemotePort;
		}
		else
		{
			src = address.Flow.RemoteAddr;
			dest = address.Flow.LocalAddr;

			srcPort = address.Flow.RemotePort;
			destPort = address.Flow.LocalPort;
		}
		
		std::cout <<"SourceAddr:\t"	<< Divert::GetIPAddress(src)	<< "\n" <<
					"SourcePort:\t"	<< srcPort						<< "\n" <<
					"DestAddr:\t"	<< Divert::GetIPAddress(dest)	<< "\n" <<
					"DestPort:\t"	<< destPort						<< "\n" <<
					"Protocol:\t"	<< (int)address.Flow.Protocol	<< "\n" <<
					"ProcessID:\t"	<< address.Flow.ProcessId		<< std::endl;
	}
}

/*BandwidthPriority::Log::LogMessage::LogMessage(LogLevel level, std::string&& errorMessage)
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
}*/
