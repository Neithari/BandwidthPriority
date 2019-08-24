#include "pch.h"
#include "WindowsHelper.h"

WindowsHelper::WindowsHelper()
{
	ConstructNetworkData();
}

const std::vector<NetworkData>& WindowsHelper::GetNetworkTableData() const
{
	return tableData;
}

std::unique_ptr<MIB_TCPTABLE2[]> WindowsHelper::InitTcpTable()
{
	auto tcpTable = std::make_unique<MIB_TCPTABLE2[]>(1);
	ULONG ulSize = sizeof(MIB_TCPTABLE2);
	unsigned int arraySize = 0;
	DWORD dwRetVal;

	// Make an initial call to GetTcpTable2 to
	// get the necessary size into the ulSize variable
	if ((dwRetVal = GetTcpTable2(tcpTable.get(), &ulSize, true)) == ERROR_INSUFFICIENT_BUFFER)
	{
		arraySize = ulSize / sizeof(MIB_TCPTABLE2);
		tcpTable = std::make_unique<MIB_TCPTABLE2[]>(arraySize);
	}
	// Make a second call to GetTcpTable2 to get
	// the actual data we require
	if ((dwRetVal = GetTcpTable2(tcpTable.get(), &ulSize, true)) == NO_ERROR)
	{
		return std::move(tcpTable);
	}

	using namespace BandwidthPriority;
	Log::log(LogLevel::Error, "Could not get the tcp table.");
	return nullptr;
}

std::wstring WindowsHelper::GetProcessPath(DWORD processID)
{
	/// TODO: Way too many fails. Need to find a better solution.
	HANDLE processHandle = NULL;
	std::wstring path = L"";

	processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, processID);
	if (processHandle != NULL)
	{
		DWORD size = MAX_PATH;
		WCHAR buffer[MAX_PATH] = L"";
		// &buffer[0] is needed to use the vector as an char string buffer
		QueryFullProcessImageNameW(processHandle, PROCESS_NAME_NATIVE, buffer, &size);
		CloseHandle(processHandle);
		path = std::wstring(buffer);
		// Get the device name, change it to a drive letter and replace it in path
		std::wstring deviceName(path.begin(), path.begin() + 23);
		deviceName = DeviceNameToDriveLetter(deviceName);
		deviceName.append(path, 23, std::wstring::npos);
		path = deviceName;
	}
	else
	{
		using namespace BandwidthPriority;
		Log::log(LogLevel::Warning, "Failed to open process.");
	}

	return std::move(path);
}

void WindowsHelper::ConstructNetworkData()
{
	/// TODO: Add IPv6 support
	/// TODO: Add UDP table

	auto tcpTable = InitTcpTable();
	// We need this pointer cast to access the underlying data the right way. If we access it like an array
	// I think the padding will corrupt the data and we get garbage results after the first.
	PMIB_TCPTABLE2 pTcpTable = static_cast<PMIB_TCPTABLE2>(tcpTable.get());
	for (int i = 0; i < (int)pTcpTable->dwNumEntries; i++)
	{
		tableData.emplace_back();
		auto srcAddr = pTcpTable->table[i].dwLocalAddr;
		auto srcPort = pTcpTable->table[i].dwLocalPort;
		auto dstAddr = pTcpTable->table[i].dwRemoteAddr;
		auto dstPort = pTcpTable->table[i].dwRemotePort;
		in_addr ipAddress;
		
		/// TODO: Look for a proof this works 100% or find another solution
		// If statement is true, then the connection is incoming, false is outgoing and we need to swap source and destination
		if (srcPort == dstPort && srcPort != 0)
		{
			DWORD tmpAddr = srcAddr;
			DWORD tmpPort = srcPort;
			srcAddr = dstAddr;
			srcPort = dstPort;
			dstAddr = tmpAddr;
			dstPort = tmpPort;
		}
		// This will convert the DWORD into a string in the normal dotted-decimal format(127.0.0.0)
		ipAddress.S_un.S_addr = (u_long)srcAddr;
		tableData.back().tuple.srcAddress = inet_ntoa(ipAddress);
		ipAddress.S_un.S_addr = (u_long)dstAddr;
		tableData.back().tuple.dstAddress = inet_ntoa(ipAddress);
		// This will convert the port number in network byte order to the port number in host byte order.
		tableData.back().tuple.srcPort = ntohs((u_short)srcPort);
		tableData.back().tuple.dstPort = ntohs((u_short)dstPort);

		tableData.back().processID = (unsigned int)pTcpTable->table[i].dwOwningPid;
		tableData.back().processPath = GetProcessPath(pTcpTable->table[i].dwOwningPid);
	}
}

std::wstring WindowsHelper::GetDriveLetter(PWCHAR volumeName) const
{
	/// TODO: Change the raw pointers to something more save
	DWORD  charCount = MAX_PATH + 1;
	PWCHAR pNames = NULL;
	bool   success = false;

	do
	{
		// Allocate a buffer to hold the paths.
		pNames = (PWCHAR) new BYTE[charCount * sizeof(WCHAR)];

		if (!pNames)
		{
			// If memory can't be allocated, return.
			return std::wstring();
		}

		// Obtain all of the paths for this volume.
		success = GetVolumePathNamesForVolumeNameW(volumeName, pNames, charCount, &charCount);

		if (GetLastError() != ERROR_MORE_DATA)
		{
			using namespace BandwidthPriority;
			Log::log(LogLevel::Info, "There is no Path.");
			// There is no spoon... I mean path.
			break;
		}
		if (!success)
		{
			// Try again with the new suggested size.
			delete[] pNames;
			pNames = NULL;
			break;
		}
	} while (!success);

	if (pNames != NULL)
	{
		std::wstring path(pNames);
		// Get rid of the "//"
		path.erase(path.end() - 1, path.end());

		delete[] pNames;
		pNames = NULL;

		return std::move(path);
	}

	using namespace BandwidthPriority;
	Log::log(LogLevel::Warning, "Could not get path");
	return std::wstring();
}

std::wstring WindowsHelper::DeviceNameToDriveLetter(const std::wstring& deviceName) const
{
	std::wstring driveLetter = L"";
	DWORD  charCount = 0;
	WCHAR  currentDeviceName[MAX_PATH] = L"";
	DWORD  error = ERROR_SUCCESS;
	HANDLE findHandle = INVALID_HANDLE_VALUE;
	size_t index = 0;
	bool   success = false;
	WCHAR  volumeName[MAX_PATH] = L"";

	// Enumerate all volumes in the system.
	findHandle = FindFirstVolumeW(volumeName, ARRAYSIZE(volumeName));

	if (findHandle == INVALID_HANDLE_VALUE)
	{
		error = GetLastError();
		std::string errorMessage = "FindFirstVolumeW failed with error code: " + std::to_string(GetLastError());
		using namespace BandwidthPriority;
		Log::log(LogLevel::Error, std::move(errorMessage));
		return driveLetter;
	}

	// QUeryDosDevie untill we get a match or an error.
	while (true)
	{
		// Skip the \\?\ prefix and remove the trailing backslash.
		index = wcslen(volumeName) - 1;

		if (volumeName[0] != L'\\' ||
			volumeName[1] != L'\\' ||
			volumeName[2] != L'?' ||
			volumeName[3] != L'\\' ||
			volumeName[index] != L'\\')
		{
			error = ERROR_BAD_PATHNAME;
			using namespace BandwidthPriority;
			Log::log(LogLevel::Error, L"FindFirstVolumeW/FindNextVolumeW returned a bad path:");
			Log::log(LogLevel::Error, std::wstring(volumeName));
			break;
		}

		//
		//  QueryDosDeviceW does not allow a trailing backslash,
		//  so temporarily remove it.
		volumeName[index] = L'\0';

		charCount = QueryDosDeviceW(&volumeName[4], currentDeviceName, ARRAYSIZE(currentDeviceName));

		volumeName[index] = L'\\';

		if (charCount == 0)
		{
			std::string errorMessage = "QueryDosDeviceW failed with error code: " + std::to_string(GetLastError());
			using namespace BandwidthPriority;
			Log::log(LogLevel::Error, std::move(errorMessage));
			break;
		}
		// If we found the matching path, save the drive letter and break
		if (deviceName[22] == currentDeviceName[22])
		{
			driveLetter = GetDriveLetter(volumeName);
			break;
		}

		// Move on to the next volume.
		success = FindNextVolumeW(findHandle, volumeName, ARRAYSIZE(volumeName));

		if (!success)
		{
			error = GetLastError();

			if (error != ERROR_NO_MORE_FILES)
			{
				std::string errorMessage = "FindNextVolumeW failed with error code: " + std::to_string(GetLastError());
				using namespace BandwidthPriority;
				Log::log(LogLevel::Error, std::move(errorMessage));
				break;
			}

			// Finished iterating through all the volumes.
			error = ERROR_SUCCESS;
			using namespace BandwidthPriority;
			Log::log(LogLevel::Warning, "No matching device name found.");
			break;
		}
	}

	FindVolumeClose(findHandle);
	findHandle = INVALID_HANDLE_VALUE;

	return driveLetter;
}
