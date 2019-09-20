/// Capture of all TCP and only outgoing UDP is needed. That is because UDP has no error checking so delaying incomming UDP
/// packets will not have the desired effect of reducing latency, but will instead result in breaking the connection using
/// them in one way or the other.

#include "pch.h"
#include "Divert.h"
#include "PacketManager.h"

int main(int argc, char** argv)
{
	std::cout << "Please enter priority programm path...(Format: C:\\foo\\bar.exe)" << std::endl;
	std::wstring priorityPath;
	std::getline(std::wcin, priorityPath);

	std::wcout << L"\n" << priorityPath << L" will be prioritized.\nPress Enter to start and again to stop!" << std::endl;
	std::cin.get();

	{
		PacketManager packetManager(priorityPath);
		std::cin.get();
	}
	using namespace BandwidthPriority;
	Log::PrintLog(LogLevel::Info);
}
