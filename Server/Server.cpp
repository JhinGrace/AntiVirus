#include "Server.h"
#include <Liba.h>
#include <thread>
#include <iostream>

Server::Server()
{
	clientUp = CreateEvent(NULL, TRUE, FALSE, TEXT("ClientUpEvent"));

	std::thread ComThread(&Server::Read, this);
	ComThread.join();
}

Server::~Server()
{
	Liba::Disconnect(hServer);
	Liba::Disconnect(hClient);
}

void Server::Read()
{
	hServer = Liba::CreateSlot(u"\\\\.\\mailslot\\server");

	while (true)
	{
		Wait();
		while (WaitForSingleObject(clientUp, 0) != WAIT_TIMEOUT)
		{
			if (!Liba::HasNewData(hServer))
				continue;

			processRequest();
		}

		Liba::Disconnect(hClient);
	}
}

void Server::processRequest()
{
	int8_t cmdCode = Liba::ReadInt8(hServer);
	if (cmdCode == (int8_t)CMDCODE::TEST)
		processTestRequest();
}

void Server::processTestRequest()
{
	//read from client
	uint32_t testNumber = Liba::ReadInt32(hServer);
	std::u16string testString = Liba::ReadU16String(hServer);

	//write in console
	std::cout << testNumber << std::endl;
	std::wcout << (wchar_t*)testString.c_str() << std::endl;


	//back to client
	Liba::WriteInt32(hClient, testNumber);
	Liba::WriteU16String(hClient, testString);

}

void Server::Wait()
{
	hClient = Liba::ConnectToSlot(u"\\\\.\\mailslot\\client");

	while (Liba::IsInvalid(hClient))
	{
		Sleep(10);
		hClient = Liba::ConnectToSlot(u"\\\\.\\mailslot\\client");
	}

	WaitForSingleObject(clientUp, INFINITE);
}