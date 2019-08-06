#include "pch.h"

#include "Divert.h"

Divert::Divert()
	:
	Divert("true", WINDIVERT_LAYER_NETWORK)
{
}

Divert::Divert(const std::string& filter,const WINDIVERT_LAYER layer, int priority, unsigned int flags)
	:
	layer(layer)
{
	/*HANDLE WinDivertOpen(
		__in const char* filter,
		__in WINDIVERT_LAYER layer,
		__in INT16 priority,
		__in UINT64 flags
	);*/
	divertHandle = WinDivertOpen(filter.c_str(), layer, priority, flags);

	if (divertHandle == INVALID_HANDLE_VALUE)
	{
		std::cerr << "DivertHandle not opened.\n";
		LogError(GetLastError());
		initialized = false;
	}
	else
	{
		std::cout << "Divert Handle with filter \"" << filter << "\" and layer \"" << layer << "\" opened" << std::endl;
		initialized = true;
	}
}

Divert::~Divert()
{
	initialized = false;
	WinDivertClose(divertHandle);
	std::cout << "Divert Handle with layer \"" << layer << "\" closed" << std::endl;
}

bool Divert::IsInitialized() const
{
	return initialized;
}

std::unique_ptr<WINDIVERT_ADDRESS> Divert::GetPacketAddress() const
{
	// Read a matching packet address.
	auto address = std::make_unique <WINDIVERT_ADDRESS>();
	if (!WinDivertRecv(divertHandle, nullptr, 0,nullptr,address.get()))
	{
		std::cerr << "warning: failed to receive packet\n";
		LogError(GetLastError());
	}
	return std::move(address);
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
	if (layer == WINDIVERT_LAYER_FLOW)
	{
		if (!WinDivertRecv(divertHandle, nullptr, 0, nullptr, &packet->address))
		{
			std::cerr << "warning: failed to read packet\n";
			LogError(GetLastError());
		}

		const UINT32* src, * dest;
		UINT16 srcPort, destPort;
		if (packet->address.Outbound == 1)
		{
			src = packet->address.Flow.LocalAddr;
			dest = packet->address.Flow.RemoteAddr;

			srcPort = packet->address.Flow.LocalPort;
			destPort = packet->address.Flow.RemotePort;
		}
		else
		{
			src = packet->address.Flow.RemoteAddr;
			dest = packet->address.Flow.LocalAddr;

			srcPort = packet->address.Flow.RemotePort;
			destPort = packet->address.Flow.LocalPort;
		}

		packet->tuple.srcAddress = GetIPAddress(src);
		packet->tuple.srcPort = srcPort;
		packet->tuple.dstAddress = GetIPAddress(dest);
		packet->tuple.dstPort = destPort;
		packet->tuple.protocol = packet->address.Flow.Protocol;
	}
	else if (layer == WINDIVERT_LAYER_NETWORK)
	{
		if (!WinDivertRecv(divertHandle, packet->packetData, sizeof(packet->packetData), &packet->packetLength, &packet->address))
		{
			std::cerr << "warning: failed to read packet\n";
			LogError(GetLastError());
		}
		Header header(*packet);

		packet->tuple.srcAddress = header.GetSource();
		packet->tuple.srcPort = header.GetSourcePort();
		packet->tuple.dstAddress = header.GetDestination();
		packet->tuple.dstPort = header.GetDestinationPort();
		packet->tuple.protocol = header.protocol;
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
	if (!WinDivertSend(divertHandle, packet.packetData, packet.packetSize, &packet.packetLength, &packet.address) && layer == WINDIVERT_LAYER_NETWORK)
	{
		std::cerr << "warning: failed to reinject packet\n";
		LogError(GetLastError());
	}
}

std::string Divert::GetIPAddress(UINT32 address)
{
	char buffer[128];
	WinDivertHelperFormatIPv4Address(address, buffer, sizeof(buffer));
	return std::move(std::string(buffer));
}

std::string Divert::GetIPAddress(const UINT32* address)
{
	char buffer[128];
	WinDivertHelperFormatIPv6Address(address, buffer, sizeof(buffer));
	return std::move(std::string(buffer));
}

void Divert::LogError(const DWORD& errorCode) const
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

Header::Header(const Packet& packet)
{
	//BOOL WinDivertHelperParsePacket(
	//	__in PVOID pPacket,
	//	__in UINT packetLen,
	//	__out_opt PWINDIVERT_IPHDR * ppIpHdr,
	//	__out_opt PWINDIVERT_IPV6HDR * ppIpv6Hdr,
	//	__out_opt UINT8 * pProtocol,
	//	__out_opt PWINDIVERT_ICMPHDR * ppIcmpHdr,
	//	__out_opt PWINDIVERT_ICMPV6HDR * ppIcmpv6Hdr,
	//	__out_opt PWINDIVERT_TCPHDR * ppTcpHdr,
	//	__out_opt PWINDIVERT_UDPHDR * ppUdpHdr,
	//	__out_opt PVOID * ppData,
	//	__out_opt UINT * pDataLen,
	//	__out_opt PVOID * ppNext,
	//	__out_opt UINT * pNextLen
	//);
	WinDivertHelperParsePacket(packet.GetData(), packet.GetLength(), &ip_header, &ipv6_header, &protocol,
		&icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL, NULL, NULL, NULL);
}

std::string Header::GetSource()
{
	if (ip_header)
	{
		return Divert::GetIPAddress(WinDivertHelperNtohl(ip_header->SrcAddr));
	}
	if (ipv6_header)
	{
		UINT32 srcAddr[4];
		WinDivertHelperNtohIPv6Address(ipv6_header->SrcAddr, srcAddr);
		return Divert::GetIPAddress(srcAddr);
	}
	return std::string();
}

std::string Header::GetDestination()
{
	if (ip_header)
	{
		return Divert::GetIPAddress(WinDivertHelperNtohl(ip_header->DstAddr));
	}
	if (ipv6_header)
	{
		UINT32 dstAddr[4];
		WinDivertHelperNtohIPv6Address(ipv6_header->DstAddr, dstAddr);
		return Divert::GetIPAddress(dstAddr);
	}
	return std::string();
}

UINT16 Header::GetSourcePort()
{
	if (tcp_header)
	{
		return ntohs(tcp_header->SrcPort);
	}
	if (udp_header)
	{
		return ntohs(udp_header->SrcPort);
	}
	return 0;
}

UINT16 Header::GetDestinationPort()
{
	if (tcp_header)
	{
		return ntohs(tcp_header->DstPort);
	}
	if (udp_header)
	{
		return ntohs(udp_header->DstPort);
	}
	return 0;
}

UINT8 Header::GetVersion()
{
	if (ip_header)
	{
		return ip_header->Version;
	}
	if (ipv6_header)
	{
		return ipv6_header->Version;
	}
	return 0;
}

UINT16 Header::GetLength()
{
	if (ip_header)
	{
		return ip_header->Length;
	}
	if (ipv6_header)
	{
		return ipv6_header->Length;
	}
	return 0;
}
