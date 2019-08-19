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

const std::wstring& Packet::GetProcessPath() const
{
	return networkData.processPath;
}

void Packet::SetProcessPath(std::wstring&& path)
{
	networkData.processPath = path;
}

const DWORD Packet::GetProcessId() const
{
	return networkData.processID;
}

void Packet::SetProcessId(DWORD processId)
{
	networkData.processID = processId;
}

bool Packet::IsMatching(const Packet& other) const
{
	return networkData.tuple.IsMatching(other.networkData.tuple);
}

bool NetworkTuple::IsMatching(const NetworkTuple& other) const
{
	return  srcAddress.compare(other.srcAddress) == 0 && srcPort == other.srcPort &&
			dstAddress.compare(other.dstAddress) == 0 && dstPort == other.dstPort &&
			protocol == protocol;
}
