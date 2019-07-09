#pragma once
#include "windivert.h"
#include <vector>
#include <memory>

#define MAXBUFFER 0xFFFF

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

private:
	friend class Divert;

	char packetData[MAXBUFFER];
	unsigned int packetSize = MAXBUFFER;
	unsigned int packetLength;
	WINDIVERT_ADDRESS address;
};

