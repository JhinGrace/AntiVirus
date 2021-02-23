#pragma once
#include <Liba.h>

class Server
{
public:
	Server();
	~Server();

private:
	void Read();
	void Wait();
	void processRequest();
	void processTestRequest();

private:
	HANDLE hClient;
	HANDLE hServer;
	HANDLE clientUp;
};