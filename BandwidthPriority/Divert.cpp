#include "Divert.h"
#include <vector>

Divert::Divert()
	:
	Divert("true")
{
}

Divert::Divert(std::string filter)
{
	divertHandle = WinDivertOpen(filter.c_str(), WINDIVERT_LAYER_NETWORK, 0, 0);

	if (divertHandle == INVALID_HANDLE_VALUE)
	{
		DWORD lastError = GetLastError();
		if (lastError == ERROR_INVALID_PARAMETER)
		{
			std::cerr << "Failed to start filtering: invalid filter syntax." << std::endl;
		}
		else
		{
			std::cerr << "Failed to open the WinDivert device (code:" << lastError << ").\n" <<
				"Make sure you run as Administrator." << std::endl;
		}
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
		std::cerr << "warning: failed to read packet (" << GetLastError() << ")\n";
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
		std::cerr << "warning: failed to reinject packet (" << GetLastError() << ")\n";
	}
}
