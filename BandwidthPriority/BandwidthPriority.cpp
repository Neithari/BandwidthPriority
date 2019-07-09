#include <Windows.h>
#include <iostream>
#include <string>
#include <vector>

#include "windivert.h"
#include "Divert.h"
#include "Packet.h"


int main(int argc, char** argv)
{
	Divert allPacketsHandle;
	std::vector<std::unique_ptr<Packet>> packets;
	packets.reserve(128);

	if (allPacketsHandle.IsInitialized())
	{
		std::cout << "Capturing Packets:" << std::endl;
		for (int i = 0; i < 128; i++)
		{
			packets.push_back(allPacketsHandle.GetPacket());
			std::cout << "Packet number: " << packets.size() << " Packet timestamp: " << packets.back()->GetAddress().Timestamp << std::endl;
		}

		std::cout << "Sending Packets:" << std::endl;
		int i = 1;
		for (auto& p : packets)
		{
			std::cout << "Packet number: " << i++ << " Packet timestamp: " << p->GetAddress().Timestamp << std::endl;
			allPacketsHandle.SendPacket(*p);
		}
		packets.clear();
		std::cout << "Packet size: " << packets.size() << std::endl;
	}
}
