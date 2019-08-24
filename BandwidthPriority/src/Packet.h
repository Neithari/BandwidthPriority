#pragma once
#define MAXBUFFER 0xFFFF

struct NetworkTuple
{
	bool IsMatching(const NetworkTuple& other) const;

	bool operator==(const NetworkTuple& other) const;

	std::string srcAddress = "";
	UINT16 srcPort = 0;
	std::string dstAddress = "";
	UINT16 dstPort = 0;
	UINT8 protocol = 0;
};

struct NetworkData
{
	NetworkTuple tuple;
	unsigned long processID = 0;
	std::wstring processPath = L"";
};

class Packet
{
public:	
	Packet() = default;
	Packet(const Packet& other) = delete;
	Packet& operator=(const Packet& other) = delete;

	const char* GetData() const;
	unsigned int GetSize() const;
	unsigned int GetLength() const;
	const WINDIVERT_ADDRESS& GetAddress() const;
	const std::wstring& GetProcessPath() const;
	void SetProcessPath(std::wstring& path);
	const DWORD GetProcessId() const;
	void SetProcessId(DWORD processId);
	const NetworkData& GetNetworkData() const;
	// Will check if the NetworkTuples are the same.
	bool IsMatching(const Packet& other) const;
	bool IsMatching(const NetworkTuple& other) const;

	// Use only in PacketManager GatherProcessData!
	NetworkData&& PilferNetworkData();

private:
	friend class Divert;

	char packetData[MAXBUFFER];
	unsigned int packetSize = MAXBUFFER;
	unsigned int packetLength = 0;
	WINDIVERT_ADDRESS address;
	NetworkData networkData;
};
