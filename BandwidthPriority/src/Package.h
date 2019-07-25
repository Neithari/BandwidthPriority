#pragma once
#include "windivert.h"
#include <vector>

class Packet
{
public:
	Packet(std::vector<unsigned int>& data, unsigned int size, unsigned int length, WINDIVERT_ADDRESS address);

	std::vector<unsigned char> GetData() const;
	unsigned int GetSize() const;
	unsigned int GetLength() const;
	WINDIVERT_ADDRESS GetAddress() const;

private:
	std::vector<unsigned char> packetData;
	unsigned int packetSize;
	unsigned int packetLength;
	WINDIVERT_ADDRESS address;
};

