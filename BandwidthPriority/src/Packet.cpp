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

void Packet::SetProcessPath(std::wstring& path)
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

const NetworkData& Packet::GetNetworkData() const
{
	return networkData;
}

bool Packet::IsMatching(const Packet& other) const
{
	return IsMatching(other.networkData.tuple);
}

bool Packet::IsMatching(const NetworkTuple& other) const
{
	return networkData.tuple.IsMatching(other);
}

NetworkData&& Packet::PilferNetworkData()
{
	return std::move(networkData);
}

bool NetworkTuple::IsMatching(const NetworkTuple& other) const
{
	return  *this == other;
}

bool NetworkTuple::operator==(const NetworkTuple& other) const
{
	return (srcAddress	== other.srcAddress &&
			srcPort		== other.srcPort	&&
			dstAddress	== other.dstAddress &&
			dstPort		== other.dstPort	&&
			protocol	== other.protocol);

}
