#include "pch.h"

#include "Divert.h"
#include "Packet.h"


int main(int argc, char** argv)
{
	Divert idHandle("true", WINDIVERT_LAYER_FLOW, 2, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY);
	Divert pHandle("true", WINDIVERT_LAYER_NETWORK, 1);
	Divert packetHandle("true", WINDIVERT_LAYER_NETWORK);
	std::vector<std::unique_ptr<Packet>> packets;
	packets.reserve(10);

	for (int i = 0; i < 10; i++)
	{
		if (idHandle.IsInitialized())
		{
			std::unique_ptr<WINDIVERT_ADDRESS> address = idHandle.ReadPacketAddress();
			std::cout << "Process ID: " << address->Flow.ProcessId <<
				"\nTimestamp: " << address->Timestamp <<
				"\nOutbound?: " << address->Outbound <<
				"\nIPChecksumm: " << address->IPChecksum << std::endl;

			UINT32* src, *dest;
			if (address->Outbound == 1)
			{
				src = address->Flow.LocalAddr;
				dest = address->Flow.RemoteAddr;
			}
			else
			{
				src = address->Flow.RemoteAddr;
				dest = address->Flow.LocalAddr;
			}
			std::string source = pHandle.GetIPAddress(src);
			std::string destination = pHandle.GetIPAddress(dest);
			std::cout << "Source address: " << source <<
				"\nDestination address: " << destination << std::endl;
		}
		if (pHandle.IsInitialized())
		{

			PWINDIVERT_IPHDR ip_header;
			PWINDIVERT_IPV6HDR ipv6_header;
			PWINDIVERT_TCPHDR tcp_header;
			PWINDIVERT_UDPHDR udp_header;
			PWINDIVERT_ICMPHDR icmp_header;
			PWINDIVERT_ICMPV6HDR icmpv6_header;
			UINT8 protocol;

			auto packet = pHandle.GetPacket();
			auto address = packet->GetAddress();
			std::cout << "Timestamp: " << address.Timestamp <<
				"\nOutbound?: " << address.Outbound <<
				"\nIPChecksumm: " << address.IPChecksum << std::endl;

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
			WinDivertHelperParsePacket(packet->GetData(), packet->GetLength(), &ip_header, &ipv6_header, &protocol,
				&icmp_header, &icmpv6_header, &tcp_header, &udp_header, NULL, NULL, NULL, NULL);
			if (ip_header)
			{
				std::string src = pHandle.GetIPAddress(ip_header->SrcAddr);
				std::string dest = pHandle.GetIPAddress(ip_header->DstAddr);
				std::cout << "Source address: " << src <<
					"\nDestination address: " << dest <<
					"\nTTL: " << (unsigned int)ip_header->TTL << std::endl;
			}
			if (ipv6_header)
			{
				std::string src = pHandle.GetIPAddress(ipv6_header->SrcAddr);
				std::string dest = pHandle.GetIPAddress(ipv6_header->DstAddr);
				std::cout << "Source address: " << src <<
					"\nDestination address: " << dest <<
					"\nIPv6 " << std::endl;
			}

			std::cout << "Sending Packet:" << std::endl;
			pHandle.SendPacket(*packet);
		}
		if (packetHandle.IsInitialized())
		{
			auto packet = packetHandle.GetPacket();
			auto address = packet->GetAddress();
			std::cout << "Timestamp: " << address.Timestamp <<
				"\nOutbound?: " << address.Outbound <<
				"\nIPChecksumm: " << address.IPChecksum << std::endl;

			std::cout << "Sending Packet:" << std::endl;
			packetHandle.SendPacket(*packet);
		}
	}
}
