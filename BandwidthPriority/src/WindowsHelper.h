#pragma once
#include "Packet.h"

class WindowsHelper
{
public:
	WindowsHelper();
	
	const std::vector<NetworkData>& GetNetworkTableData() const;
private:
	// Initialize the tcp table. Used to construct the NetworkData vector
	std::unique_ptr<MIB_TCPTABLE2[]> InitTcpTable();
	std::wstring GetProcessPath(DWORD processID);
	void ConstructNetworkData();
	std::wstring GetDriveLetter(__in PWCHAR volumeName) const;
	std::wstring DeviceNameToDriveLetter(const std::wstring& deviceName) const;
private:
	std::vector<NetworkData> tableData;
};

