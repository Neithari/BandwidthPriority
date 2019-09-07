#include "pch.h"
#include "PacketManager.h"

PacketManager::PacketManager(const std::wstring& priorityPath)
	:
	priorityPath(priorityPath),
	t_gather(&PacketManager::GatherProcessData, this),
	t_getPackets(&PacketManager::GetPacketsWorker, this, "true"),
	t_sendPackets(&PacketManager::SendPacketsWorker, this)
{
}

PacketManager::~PacketManager()
{
	stopThreads = true;

	t_gather.join();
	t_getPackets.join();
	t_sendPackets.join();
}

void PacketManager::StopPacketManager()
{
	stopThreads = true;
}

bool PacketManager::PacketIsFromPriority(const Packet& packet) const
{
	return priorityPath == packet.GetProcessPath();
}

void PacketManager::GetPacketsWorker(std::string filter)
{
	/// TODO: When the worker stops packets already captured inside WINDIVERT will be lost.
	/// To fix this the whole tree must return null when there is no packet in recv and there
	/// needs to be a function in divert to call WinDivertShutdown.
	Divert getHandle(filter, WINDIVERT_LAYER_NETWORK, 10, WINDIVERT_FLAG_RECV_ONLY);

	if (getHandle.IsInitialized())
	{
		while (!stopThreads)
		{
			auto packet = GetPacket(getHandle);
			// print the packet details for debug
			//std::cout << "Get packet:" << std::endl;
			//BandwidthPriority::Log::PrintHeaderDetails(*packet);

			SetPacketPath(*packet);
			// print the path for debug
			//std::wcout << L"Path: " << packet->GetProcessPath() << std::endl;

			if (PacketIsFromPriority(*packet))
			{
				std::lock_guard<std::mutex> lock(mtx);
				priorityQue.push_back(std::move(packet));
			}
			else
			{
				std::lock_guard<std::mutex> lock(mtx);
				normalQue.push_back(std::move(packet));
			}
		}
		using namespace BandwidthPriority;
		Log::log(LogLevel::Info, "GetPacketsWorker stopped without errors.");
	}
	else
	{
		using namespace BandwidthPriority;
		Log::log(LogLevel::Error, "Handle in GetPacketsWorker is not initialized.");
	}
}

void PacketManager::SendPacketsWorker()
{
	Divert sendHandle("true", WINDIVERT_LAYER_NETWORK, 0, WINDIVERT_FLAG_SEND_ONLY);
	if (sendHandle.IsInitialized())
	{
		while (!stopThreads)
		{
			if (!priorityQue.empty())
			{
				std::lock_guard<std::mutex> lock(mtx);
				// print the packet details for debug
				std::cout << "Send packet priority:" << std::endl;
				//BandwidthPriority::Log::PrintHeaderDetails(*priorityQue.back());
				// print the path for debug
				//std::wcout << L"Path: " << priorityQue.back()->GetProcessPath() << std::endl;

				sendHandle.SendPacket(*priorityQue.back());
				priorityQue.pop_back();
			}
			else if (!normalQue.empty())
			{
				std::lock_guard<std::mutex> lock(mtx);
				// print the packet details for debug
				std::cout << "Send packet normal:" << std::endl;
				//BandwidthPriority::Log::PrintHeaderDetails(*normalQue.back());
				// print the path for debug
				//std::wcout << L"Path: " << normalQue.back()->GetProcessPath() << std::endl;

				sendHandle.SendPacket(*normalQue.back());
				normalQue.pop_back();
			}
		}
		// Send the remaining packets to prepare to stop the thread
		while (!priorityQue.empty())
		{
			std::lock_guard<std::mutex> lock(mtx);
			// print the packet details for debug
			std::cout << "Send packet priority after stop:" << std::endl;
			//BandwidthPriority::Log::PrintHeaderDetails(*priorityQue.back());
			// print the path for debug
			//std::wcout << L"Path: " << priorityQue.back()->GetProcessPath() << std::endl;

			sendHandle.SendPacket(*priorityQue.back());
			priorityQue.pop_back();
		}
		while (!normalQue.empty())
		{
			std::lock_guard<std::mutex> lock(mtx);
			// print the packet details for debug
			std::cout << "Send packet normal after stop:" << std::endl;
			//BandwidthPriority::Log::PrintHeaderDetails(*normalQue.back());
			// print the path for debug
			//std::wcout << L"Path: " << normalQue.back()->GetProcessPath() << std::endl;

			sendHandle.SendPacket(*normalQue.back());
			normalQue.pop_back();
		}
		using namespace BandwidthPriority;
		Log::log(LogLevel::Info, "SendPacketsWorker stopped without errors.");
	}
	else
	{
		using namespace BandwidthPriority;
		Log::log(LogLevel::Error, "Handle in SendPacketsWorker is not initialized.");
	}
}

std::unique_ptr<Packet> PacketManager::GetPacket(Divert& handle)
{
	return handle.GetPacket();
}

bool PacketManager::SetPacketPath(Packet& packet)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto search = pathMap.find(packet.GetNetworkData().tuple);
	if (search != pathMap.end())
	{
		packet.SetProcessPath(search->second);
		return true;
	}
	return false;
}

void PacketManager::GetNetworkTableData()
{
	auto table = windows.GetNetworkTableData();

	std::lock_guard<std::mutex> lock(mtx);
	for (auto& t : table)
	{
		// Only add it to the pathMap if the path is not empty.
		if (t.processPath == L"")
			continue;

		// Create a reverse version of tuple to insert in pathMap because Flows only show one direction
		// and the answer packets not recognizing the correct path. Till I find another solution.
		/// TODO: Find a better solution for this problem. Pref. without writing a custom unordered map search function.
		NetworkTuple reverseTuple = ReverseTuple(t.tuple);

		pathMap.insert({ t.tuple, t.processPath });
		pathMap.insert({ reverseTuple, t.processPath });
	}
	using namespace BandwidthPriority;
	std::string info = "Size of pathMap: " + std::to_string(pathMap.size());
	Log::log(LogLevel::Debug, std::move(info));
}

void PacketManager::GatherProcessData()
{
	GetNetworkTableData();
	// Print pathMap for debug
	/*for (auto& it : pathMap)
	{
		BandwidthPriority::Log::PrintNetworkTuple(it.first);
		std::wcout << L"Path:\t" << it.second << std::endl;
	}*/

	// Open a handle on flow layer to get all packets with process id to match them with other packets via network tuple.
	Divert flowHandle("true", WINDIVERT_LAYER_FLOW, 100, WINDIVERT_FLAG_SNIFF | WINDIVERT_FLAG_RECV_ONLY);
	
	if (!flowHandle.IsInitialized())
	{
		using namespace BandwidthPriority;
		Log::log(LogLevel::Error, "Flow handle could not be opened inside GatherProcesData!");
		return;
	}

	while (!stopThreads)
	{
		auto packet = flowHandle.GetPacket();
		if (packet)
		{
			auto networkData = packet->PilferNetworkData();
			// Only add it to the pathMap if the path is not empty.
			if (networkData.processPath == L"")
				continue;

			// print networkData for debug
			//std::cout << "PathMap:" << "\n";
			//BandwidthPriority::Log::PrintNetworkTuple(networkData.tuple);
			//std::wcout << L"Path:\t" << networkData.processPath << std::endl;

			// Create a reverse version of tuple to insert in pathMap because Flows only show one direction
			// and the answer packets not recognizing the correct path. Till I find another solution.
			/// TODO: Find a better solution for this problem. Pref. without writing a custom unordered map search function.
			NetworkTuple reverseTuple = ReverseTuple(networkData.tuple);

			std::lock_guard<std::mutex> lock(mtx);
			pathMap.insert({ networkData.tuple, networkData.processPath });
			pathMap.insert({ reverseTuple, networkData.processPath });
			// Print pathMap for debug
			/*std::cout << "PathMap:" << "\n";
			for (auto& it : pathMap)
			{
				BandwidthPriority::Log::PrintNetworkTuple(it.first);
				std::wcout << L"Path:\t" << it.second << std::endl;
			}*/
			using namespace BandwidthPriority;
			std::string info = "Size of pathMap: " + std::to_string(pathMap.size());
			Log::log(LogLevel::Debug, std::move(info));
		}
	}
}

NetworkTuple PacketManager::ReverseTuple(const NetworkTuple& other) const
{
	NetworkTuple reverseTuple = other;

	reverseTuple.srcAddress = other.dstAddress;
	reverseTuple.srcPort = other.dstPort;
	reverseTuple.dstAddress = other.srcAddress;
	reverseTuple.dstPort = other.srcPort;
	return reverseTuple;
}
