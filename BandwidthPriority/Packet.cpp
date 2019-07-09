#include "Packet.h"

const char* Packet::GetData() const
{
	return packetData;
}

unsigned int Packet::GetSize() const
{
	return packetSize;
}

unsigned int Packet::GetLength() const
{
	return packetLength;
}

const WINDIVERT_ADDRESS& Packet::GetAddress() const
{
	return address;
}
