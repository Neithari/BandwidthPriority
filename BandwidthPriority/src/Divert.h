#pragma once
#include "Packet.h"

class Divert
{
public:
	Divert();
	Divert(const std::string& filter, const WINDIVERT_LAYER layer, int priority = 0, unsigned int flags = 0);

	Divert(const Divert& other) = delete;
	Divert(Divert&& other) = delete;
	~Divert();
	Divert& operator=(const Divert& other) = delete;
	Divert& operator=(Divert&& other) = delete;
	
	bool IsInitialized() const;
	std::unique_ptr<WINDIVERT_ADDRESS> ReadPacketAddress();
	std::unique_ptr<Packet> GetPacket();
	void SendPacket(Packet& packet);
	std::string GetIPAddress(UINT32 address);
	std::string GetIPAddress(const UINT32* address);

private:
	void LogError(const DWORD& errorCode);

private:
	HANDLE divertHandle;
	bool initialized = false;
};

