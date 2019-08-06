#include "pch.h"

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

bool Packet::IsMatching(const Packet& other) const
{
	return tuple.IsMatching(other.tuple);
}

bool NetworkTuple::IsMatching(const NetworkTuple& other) const
{
	return  srcAddress.compare(other.srcAddress) == 0 && srcPort == other.srcPort &&
			dstAddress.compare(other.dstAddress) == 0 && dstPort == other.dstPort &&
			protocol == protocol;
}
