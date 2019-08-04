#include "pch.h"

#include "Divert.h"
#include "Packet.h"

void PrintAddressDetails(const WINDIVERT_ADDRESS& address)
{
	std::cout <<	//"Timestamp: "	<< address.Timestamp	<< "\n" <<
					//"Outbound: "	<< address.Outbound		<< "\n" <<
					//"IPv6: "		<< address.IPv6			<< "\n" <<
					//"IPChecksum: "	<< address.IPChecksum	<< "\n" <<
					//"TCPChecksum: " << address.TCPChecksum	<< "\n" <<
					//"UDPChecksum: " << address.UDPChecksum	<< std::endl;
		"Outbound: " << address.Outbound << std::endl;
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
					//"Length: "		<< header.GetLength()			<< "\n" <<
					"Version: "		<< (int)header.GetVersion()		<< std::endl;
}

bool stopThreads = false;
std::vector<std::unique_ptr<Packet>> DivertWorker(std::string filter, WINDIVERT_LAYER layer, int priority = 0, unsigned int flags = 0)
{
	Divert handle(filter, layer, priority, flags);
	std::vector<std::unique_ptr<Packet>> packets;
	if (handle.IsInitialized())
	{
		packets.reserve(128);
		while (!stopThreads)
		{
			packets.push_back(handle.GetPacket());
			if (layer == WINDIVERT_LAYER_NETWORK)
			{
				handle.SendPacket(*packets.back());
			}
		}
	}
	return std::move(packets);
}

int main(int argc, char** argv)
{
	// std::future to get return from threads
	auto flow = std::async(DivertWorker, "true", WINDIVERT_LAYER_FLOW, 2, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY);
	auto network = std::async(DivertWorker, "true", WINDIVERT_LAYER_NETWORK, 1, 0U);

	std::cin.get();
	stopThreads = true;
	auto flowPackets = flow.get();
	auto networkPackets = network.get();

	std::cout << "Flow Layer" << "\n" <<
				 "----------" << std::endl;
	for (auto& p : flowPackets)
	{
		PrintAddressDetails(p->GetAddress());
	}
	std::cout << "\n" << "Network Layer:" << "\n" <<
						 "--------------" << std::endl;
	for (auto& p : networkPackets)
	{
		PrintAddressDetails(p->GetAddress());
		PrintHeaderDetails(*p);
	}
}
