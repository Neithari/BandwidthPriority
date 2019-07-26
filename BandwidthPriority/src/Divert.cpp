#include "Divert.h"
#include <vector>

Divert::Divert()
	:
	Divert("true")
{
}

Divert::Divert(const std::string& filter, int priority)
{
	/*HANDLE WinDivertOpen(
		__in const char* filter,
		__in WINDIVERT_LAYER layer,
		__in INT16 priority,
		__in UINT64 flags
	);*/
	divertHandle = WinDivertOpen(filter.c_str(), WINDIVERT_LAYER_NETWORK, priority, 0);

	if (divertHandle == INVALID_HANDLE_VALUE)
	{
		std::cerr << "DivertHandle not opened.\n";
		LogError(GetLastError());
		initialized = false;
	}
	else
	{
		std::cout << "Divert Handle with filter \"" << filter << "\" opened" << std::endl;
		initialized = true;
	}

	// TODO: Implement threading
}

Divert::~Divert()
{
	initialized = false;
	WinDivertClose(divertHandle);
}

bool Divert::IsInitialized() const
{
	return initialized;
}

std::unique_ptr<Packet> Divert::GetPacket()
{
	/*BOOL WinDivertRecv(
	__in HANDLE handle,
		__out_opt PVOID pPacket,
		__in UINT packetLen,
		__out_opt UINT* pRecvLen,
		__out_opt WINDIVERT_ADDRESS* pAddr
		);*/
	auto packet = std::make_unique<Packet>();

	// Read a matching packet.
	if (!WinDivertRecv(divertHandle, packet->packetData, sizeof(packet->packetData), &packet->packetLength, &packet->address))
	{
		std::cerr << "warning: failed to read packet\n";
		LogError(GetLastError());
	}
	return std::move(packet);
}



void Divert::SendPacket(Packet& packet)
{
	/*BOOL WinDivertSend(
		__in HANDLE handle,
		__in const VOID * pPacket,
		__in UINT packetLen,
		__out_opt UINT * pSendLen,
		__in const WINDIVERT_ADDRESS * pAddr
	);*/
	if (!WinDivertSend(divertHandle, packet.packetData, packet.packetSize, &packet.packetLength, &packet.address))
	{
		std::cerr << "warning: failed to reinject packet\n";
		LogError(GetLastError());
	}
}

void Divert::LogError(const DWORD& errorCode)
{
	switch (errorCode)
	{
		//WinDivertOpen
	case ERROR_FILE_NOT_FOUND:
		std::cerr << "The driver files WinDivert32.sys or WinDivert64.sys were not found." << std::endl;
		break;
	case ERROR_ACCESS_DENIED:
		std::cerr << "You need Administrator privileges to run this application." << std::endl;
		break;
	case ERROR_INVALID_PARAMETER:
		std::cerr << "Failed to start filtering: invalid filter syntax." << std::endl;
		break;
	case ERROR_INVALID_IMAGE_HASH:
		std::cerr << "The WinDivert32.sys or WinDivert64.sys driver does not have a valid digital signature." << std::endl;
		break;
	case ERROR_DRIVER_FAILED_PRIOR_UNLOAD:
		std::cerr << "An incompatible version of the WinDivert driver is currently loaded." << std::endl;
		break;
	case ERROR_SERVICE_DOES_NOT_EXIST:
		std::cerr << "The handle was opened with the WINDIVERT_FLAG_NO_INSTALL flag and the WinDivert driver is not already installed." << std::endl;
		break;
	case ERROR_DRIVER_BLOCKED:
		std::cerr << "Failed to open the WinDivert device because the driver was blocked." << std::endl;
		break;
	case EPT_S_NOT_REGISTERED:
		std::cerr << "The Base Filtering Engine service has been disabled." << std::endl;
		break;
		// WinDivertRecv
	case ERROR_INSUFFICIENT_BUFFER:
		std::cerr << "The captured packet is larger than the packet buffer." << std::endl;
		break;
	case ERROR_NO_DATA:
		std::cerr << "The handle has been shutdown using WinDivertShutdown() and the packet queue is empty." << std::endl;
		break;
		// WinDivertSend
	case ERROR_HOST_UNREACHABLE:
		std::cerr << "An impostor packet is injected and the ip.TTL or ipv6.HopLimit field is 0.\n" <<
			"Refused to send to not get stuck in a infinite loop caused by impostor packets." << std::endl;
		break;
	default:
		std::cerr << "An unknown error occured (code:" << errorCode << ").\n" << std::endl;
		break;
	}
}
