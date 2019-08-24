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
			SetPacketPath(*packet);

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
				sendHandle.SendPacket(*priorityQue.back());
				priorityQue.pop_back();
			}
			else if (!normalQue.empty())
			{
				std::lock_guard<std::mutex> lock(mtx);
				sendHandle.SendPacket(*normalQue.back());
				normalQue.pop_back();
			}
		}
		// Send the remaining packets to prepare to stop the thread
		while (!priorityQue.empty())
		{
			std::lock_guard<std::mutex> lock(mtx);
			sendHandle.SendPacket(*priorityQue.back());
			priorityQue.pop_back();
		}
		while (!normalQue.empty())
		{
			std::lock_guard<std::mutex> lock(mtx);
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
		pathMap.insert({ t.tuple, t.processPath });
	}
	using namespace BandwidthPriority;
	std::string info = "Size of pathMap: " + std::to_string(pathMap.size());
	Log::log(LogLevel::Debug, std::move(info));
}

void PacketManager::GatherProcessData()
{
	GetNetworkTableData();

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
			std::lock_guard<std::mutex> lock(mtx);
			pathMap.insert({ networkData.tuple, networkData.processPath });
			using namespace BandwidthPriority;
			std::string info = "Size of pathMap: " + std::to_string(pathMap.size());
			Log::log(LogLevel::Debug, std::move(info));
		}
	}
}
