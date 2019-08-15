/// Capture of all TCP and only outgoing UDP is needed. That is because UDP has no error checking so delaying incomming UDP
/// packets will not have the desired effect of reducing latency, but will instead result in breaking the connection using
/// them in one way or the other.

#include "pch.h"

#include "Divert.h"
#include "Packet.h"
#include "WindowsHelper.h"

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
						"ProcessID: "	<< address.Flow.ProcessId		<< std::endl;
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
					//"Length: "	<< header.GetLength()			<< "\n" <<
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

void PrintTcpTable()
{
	auto tcpTable = std::make_unique<MIB_TCPTABLE2[]>(1);
	ULONG ulSize = sizeof(MIB_TCPTABLE2);
	unsigned int arraySize = 0;
	DWORD dwRetVal;

	// Make an initial call to GetTcpTable2 to
	// get the necessary size into the ulSize variable
	if ((dwRetVal = GetTcpTable2(tcpTable.get(), &ulSize, true)) == ERROR_INSUFFICIENT_BUFFER)
	{
		arraySize = ulSize / sizeof(MIB_TCPTABLE2);
		tcpTable = std::make_unique<MIB_TCPTABLE2[]>(arraySize);
	}
	// Make a second call to GetTcpTable2 to get
	// the actual data we require
	if ((dwRetVal = GetTcpTable2(tcpTable.get(), &ulSize, true)) == NO_ERROR)
	{
		// We need this pointer cast to access the underlying data the right way. If we access it like an array
		// the padding will corrupt the data I think and we get garbage results after the first.
		PMIB_TCPTABLE2 pTcpTable = static_cast<PMIB_TCPTABLE2>(tcpTable.get());
		in_addr ipAddress;
		for (unsigned int i = 0; i < (int)pTcpTable->dwNumEntries; i++)
		{
			ipAddress.S_un.S_addr = (u_long)pTcpTable->table[i].dwLocalAddr;
			std::cout << "ID: " << pTcpTable->table[i].dwOwningPid << "\tIP: " << inet_ntoa(ipAddress) << ":" << ntohs((u_short)pTcpTable->table[i].dwLocalPort) << std::endl;
		}
	}
}

int main(int argc, char** argv)
{
	WindowsHelper network;
	auto tables = network.GetNetworkTableData();
	for (auto& t: tables)
	{
		std::cout << "ID: " << t.processID << "\tIP: " << t.tuple.dstAddress << ":" << t.tuple.dstPort << "\n";
		if (t.processID != 0)
		{
			std::wcout << t.processPath << "\n";
		}
	}

	// std::cout << "TCP Table" << std::endl;
	// PrintTcpTable();


	// std::future to get return from threads
	// auto flow = std::async(DivertWorker, "true", WINDIVERT_LAYER_FLOW, 2, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY);
	// auto network = std::async(DivertWorker, "true", WINDIVERT_LAYER_NETWORK, 1, 0U);
	// 
	// std::cin.get();
	// stopThreads = true;
	// auto flowPackets = flow.get();
	// auto networkPackets = network.get();
	// 
	// for (auto& f : flowPackets)
	// {
	// 	std::cout << "\n" << "Flow Layer" << "\n" <<
	// 						 "----------" << std::endl;
	// 	PrintAddressDetails(f->GetAddress());
	// 	
	// 	for (auto& n : networkPackets)
	// 	{
	// 		if (n->IsMatching(*f))
	// 		{
	// 			std::cout << "Matching Network Layer:" << "\n" <<
	// 						 "-----------------------" << std::endl;
	// 			PrintAddressDetails(n->GetAddress());
	// 			PrintHeaderDetails(*n);
	// 		}
	// 	}
	// }
}
