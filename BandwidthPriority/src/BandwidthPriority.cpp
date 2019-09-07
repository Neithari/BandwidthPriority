/// Capture of all TCP and only outgoing UDP is needed. That is because UDP has no error checking so delaying incomming UDP
/// packets will not have the desired effect of reducing latency, but will instead result in breaking the connection using
/// them in one way or the other.

#include "pch.h"
#include "Divert.h"
#include "PacketManager.h"

int main(int argc, char** argv)
{
	{
		PacketManager packetManager(L"C:\\Program Files\\Firefox Nightly\\firefox.exe");
		std::cin.get();
	}
	using namespace BandwidthPriority;
	Log::PrintLog(LogLevel::Info);
}
