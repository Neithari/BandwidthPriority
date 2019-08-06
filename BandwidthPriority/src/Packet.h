#pragma once
#define MAXBUFFER 0xFFFF

struct NetworkTuple
{
	bool IsMatching(const NetworkTuple& other) const;

	std::string srcAddress;
	UINT16 srcPort;
	std::string dstAddress;
	UINT16 dstPort;
	UINT8 protocol;
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
	bool IsMatching(const Packet& other) const;

private:
	friend class Divert;

	char packetData[MAXBUFFER];
	unsigned int packetSize = MAXBUFFER;
	unsigned int packetLength = 0;
	WINDIVERT_ADDRESS address;
	NetworkTuple tuple;
};
