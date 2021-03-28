#pragma once
#include <memory>
#include "Liba.h"
#include "DB.h"
#include "Viruses.h"
#include "Monitor.h"
#include <ctime>
#include "Timmer.h"


class Server {
public:
	Server();
	~Server();
	void start();
	void shutDown();

private:
	void startReading();
	void processRequest();
	void requestDel();
	void startScan();
	void timedScan();
	void loadMonitors();
	void saveMonitors();
	void loadTimmers();
	void saveTimmers();

private:
	std::shared_ptr<DB> db;
	std::shared_ptr<Comms> comms;
	std::shared_ptr<Viruses> viruses;
	std::vector<Monitor> monitors;
	std::vector<Timmer> timmers;
	Scanner scanner;
	HANDLE hReportAddress;
	bool clientShutdown = false;
};
