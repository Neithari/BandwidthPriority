#pragma once
#include "Packet.h"

struct Header
{
	Header(const Packet& packet);

	std::string GetSource();
	std::string GetDestination();
	UINT16 GetSourcePort();
	UINT16 GetDestinationPort();
	UINT8 GetVersion();
	UINT16 GetLength();

	PWINDIVERT_IPHDR ip_header;
	PWINDIVERT_IPV6HDR ipv6_header;
	PWINDIVERT_TCPHDR tcp_header;
	PWINDIVERT_UDPHDR udp_header;
	PWINDIVERT_ICMPHDR icmp_header;
	PWINDIVERT_ICMPV6HDR icmpv6_header;
	UINT8 protocol;
};

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
	std::unique_ptr<WINDIVERT_ADDRESS> GetPacketAddress() const;
	std::unique_ptr<Packet> GetPacket();
	void SendPacket(Packet& packet);

	static std::string GetIPAddress(UINT32 address);
	static std::string GetIPAddress(const UINT32* address);

private:
	void LogError(const DWORD& errorCode) const;

private:
	HANDLE divertHandle;
	WINDIVERT_LAYER layer;
	bool initialized = false;
};

