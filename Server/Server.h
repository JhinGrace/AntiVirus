#pragma once
#include <memory>
#include <Liba.h>
#include <DB.h>

class Server
{
public:
	Server();
	~Server();

	void start();

private:
	void startReading();
	void processRequest(bool& clientDown, bool& serverDown);

private:
	std::shared_ptr<DB> base;
	std::shared_ptr<Comms> comms;
};