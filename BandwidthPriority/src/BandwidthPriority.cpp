#include "pch.h"

#include "Divert.h"
#include "Packet.h"

void PrintAddressDetails(const WINDIVERT_ADDRESS& address)
{
	std::cout <<	"Timestamp: "	<< address.Timestamp	<< "\n" <<
					"Outbound: "	<< address.Outbound		<< "\n" <<
					"IPv6: "		<< address.IPv6			<< "\n" <<
					"IPChecksum: "	<< address.IPChecksum	<< "\n" <<
					"TCPChecksum: " << address.TCPChecksum	<< "\n" <<
					"UDPChecksum: " << address.UDPChecksum	<< std::endl;
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

		std::cout <<	"SourceAddr: "	<< Divert::GetIPAddress(src)	<< "\n" <<
						"SourcePort: "	<< srcPort						<< "\n" <<
						"DestAddr: "	<< Divert::GetIPAddress(dest)	<< "\n" <<
						"DestPort: "	<< destPort						<< "\n" <<
						"Protocol: "	<< (int)address.Flow.Protocol	<< "\n" <<
						"ProcessID: " << address.Flow.ProcessId			<< std::endl;
	}
}

void PrintHeaderDetails(const Packet& packet)
{
	Header header(packet);

	std::cout <<	"SourceAddr: "	<< header.GetSource()			<< "\n" <<
					"SourcePort: "	<< header.GetSourcePort()		<< "\n" <<
					"DestAddr: "	<< header.GetDestination()		<< "\n" <<
					"DestPort: "	<< header.GetDestinationPort()	<< "\n" <<
					"Protocol: "	<< (int)header.protocol			<< "\n" <<
					"Length: "		<< header.GetLength()			<< "\n" <<
					"Version: "		<< (int)header.GetVersion()		<< std::endl;
}

int main(int argc, char** argv)
{
	Divert flowHandle("true", WINDIVERT_LAYER_FLOW, 3, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY);
	Divert network1Handle("true", WINDIVERT_LAYER_NETWORK, 1);
	Divert network0Handle("true", WINDIVERT_LAYER_NETWORK);
	// std::vector<std::unique_ptr<Packet>> packets;
	// packets.reserve(10);

	for (int i = 0; i < 10; i++)
	{
		if (flowHandle.IsInitialized())
		{
			std::unique_ptr<WINDIVERT_ADDRESS> address = flowHandle.GetPacketAddress();

			std::cout << "\n" <<	"Flow Layer" << "\n" <<
									"----------" << std::endl;
			PrintAddressDetails(*address);
		}
		if (network1Handle.IsInitialized())
		{
			auto packet = network1Handle.GetPacket();

			std::cout <<	"Network Layer: Priority 1" << "\n" <<
							"-------------------------" << std::endl;
			PrintAddressDetails(packet->GetAddress());
			PrintHeaderDetails(*packet);
			
			std::cout << "Sending Packet" << std::endl;
			network1Handle.SendPacket(*packet);
		}
		if (network0Handle.IsInitialized())
		{
			auto packet = network0Handle.GetPacket();

			std::cout <<	"Network Layer: Priority 0" << "\n" <<
							"-------------------------" << std::endl;
			PrintAddressDetails(packet->GetAddress());
			PrintHeaderDetails(*packet);

			std::cout << "Sending Packet:" << std::endl;
			network0Handle.SendPacket(*packet);
		}
	}
}
