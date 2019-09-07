#pragma once
#include "Divert.h"
#include "WindowsHelper.h"
#include "Packet.h"

// Hash_combine out of boost library
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
	std::hash<T> hasher;
	seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Overloading the hash function inside std, so no extra parameter is needed in map declaration
// Combining all the data inside NetworkTuple with help of the hash_combine function of boost.
namespace std
{
	template <> struct hash<NetworkTuple>
	{
		size_t operator()(const NetworkTuple& tuple) const
		{
			auto seed = std::hash<std::string>{}(tuple.srcAddress);
			hash_combine(seed, tuple.srcPort);
			hash_combine(seed, tuple.dstAddress);
			hash_combine(seed, tuple.dstPort);
			hash_combine(seed, tuple.protocol);

			return seed;
		}
	};
}

class PacketManager
{
public:
	PacketManager(const std::wstring& priorityPath);
	PacketManager(const PacketManager& other) = delete;
	PacketManager(PacketManager&& other) = delete;
	~PacketManager();

	PacketManager& operator=(const PacketManager& other) = delete;
	PacketManager& operator=(PacketManager&& other) = delete;

	void StopPacketManager();
private:
	bool PacketIsFromPriority(const Packet& packet) const;
	void GetPacketsWorker(std::string filter);
	void SendPacketsWorker();
	std::unique_ptr<Packet> GetPacket(Divert& handle);
	bool SetPacketPath(Packet& packet);

	void GetNetworkTableData();
	// Start in thread. Will constantly update pathMap.
	void GatherProcessData();
	/// TODO: Find a better solution for this problem. Pref. without writing a custom unordered map search function.
	NetworkTuple ReverseTuple(const NetworkTuple& other) const;
private:
	std::wstring priorityPath = L"";
	// To get paths and tables
	WindowsHelper windows;
	// Divert send ques
	std::vector<std::unique_ptr<Packet>> priorityQue;
	std::vector<std::unique_ptr<Packet>> normalQue;
	// For threading
	std::mutex mtx;
	bool stopThreads = false;
	std::thread t_gather;
	std::thread t_getPackets;
	std::thread t_sendPackets;
	// Put into a lock_guard
	std::unordered_map<NetworkTuple, std::wstring> pathMap;
};
