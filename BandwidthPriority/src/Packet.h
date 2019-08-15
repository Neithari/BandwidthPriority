#pragma once
#define MAXBUFFER 0xFFFF

struct NetworkTuple
{
	bool IsMatching(const NetworkTuple& other) const;

	std::string srcAddress = "";
	UINT16 srcPort = 0;
	std::string dstAddress = "";
	UINT16 dstPort = 0;
	UINT8 protocol = 0;
};

struct NetworkData
{
	NetworkTuple tuple;
	unsigned int processID = 0;
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
	// Will check if the NetworkTuples are the same.
	bool IsMatching(const Packet& other) const;

private:
	friend class Divert;

	char packetData[MAXBUFFER];
	unsigned int packetSize = MAXBUFFER;
	unsigned int packetLength = 0;
	WINDIVERT_ADDRESS address;
	NetworkTuple tuple;
};
