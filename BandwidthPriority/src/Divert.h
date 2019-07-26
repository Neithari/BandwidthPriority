#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <memory>

#include "windivert.h"
#include "Packet.h"

class Divert
{
public:
	Divert();
	Divert(const std::string& filter, int priority = 0);

	Divert(const Divert& other) = delete;
	Divert(Divert&& other) = delete;
	~Divert();
	Divert& operator=(const Divert& other) = delete;
	Divert& operator=(Divert&& other) = delete;
	
	bool IsInitialized() const;
	std::unique_ptr<Packet> GetPacket();
	void SendPacket(Packet& packet);

private:
	void LogError(const DWORD& errorCode);

private:
	HANDLE divertHandle;
	bool initialized = false;
};

