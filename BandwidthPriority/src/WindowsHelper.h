#pragma once
#include "Packet.h"

class WindowsHelper
{
public:
	WindowsHelper();
	
	const std::vector<NetworkData>& GetNetworkTableData() const;
	/// TODO: Implement function.
	std::wstring ProcessIdToFilePath(DWORD processID) const;
private:
	// Initialize the tcp table. Used to construct the NetworkData vector
	std::unique_ptr<MIB_TCPTABLE2[]> InitTcpTable();
	void ConstructNetworkData();
	void ConstructTcpData();

	std::wstring GetProcessPath(DWORD processID);
	std::wstring GetDriveLetter(PWCHAR volumeName) const;
	std::wstring DeviceNameToDriveLetter(const std::wstring& deviceName) const;
private:
	std::vector<NetworkData> tableData;
};

