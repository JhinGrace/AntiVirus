#include <thread>
#include <CommsMail.h>
#include <Read.h>
#include <Write.h>
#include "Server.h"
#include "Scanner.h"
#include "DBLoad.h"
#include "Monitor.h"

HANDLE mutex;

Server::Server() {
	mutex = CreateMutex(NULL, FALSE, L"Mutex");
	monitors.reserve(100);
	timmers.reserve(100);
}

Server::~Server() {
	CloseHandle(mutex);
}

void Server::startReading() {
	comms = Comms::Mailslots(u"\\\\.\\mailslot\\server", u"\\\\.\\mailslot\\client");
	while (true) {
		comms->connect();
		while (!clientShutdown) {
			processRequest();
		}
		clientShutdown = false;
	}
}

void Server::processRequest() {
	Read reader(comms);
	uint8_t cmdCode = reader.readUInt8();

	if (cmdCode == (uint8_t)CMDCODE::SERVERSHUTDOWN) {
		shutDown();
	}
	else if (cmdCode == (uint8_t)CMDCODE::CLIENTSHUTDOWN) {
		saveMonitors();
		saveTimmers();
		clientShutdown = true;
	}
	else if (cmdCode == (uint8_t)CMDCODE::SCAN) {
		startScan();
	}
	else if (cmdCode == (uint8_t)CMDCODE::DEL) {
		requestDel();
	}
	else if (cmdCode == (uint8_t)CMDCODE::STOPSCAN) {
		scanner.stopScan();
		while (!scanner.scanStopped()) {
			Sleep(1);
		}
		Write writer(comms);
		bool success = true;
		writer.writeUInt8((uint8_t)success);
	}
	else if (cmdCode == (uint8_t)CMDCODE::MONITOR) {
		Read reader(comms);
		std::u16string path = reader.readU16String();
		monitors.push_back(Monitor(path, db, viruses));

		std::thread monitorThread(&Monitor::start, std::ref(monitors[monitors.size() - 1]));
		monitorThread.detach();
	}
	else if (cmdCode == (uint8_t)CMDCODE::STOPMONITOR) {
		Read reader(comms);
		uint64_t index = reader.readUInt64();
		monitors[index].stop();

		monitors.erase(monitors.begin() + index);
	}
	else if (cmdCode == (uint8_t)CMDCODE::TIMESCAN) {
		timedScan();
	}
	else if (cmdCode == (uint8_t)CMDCODE::STOPTIMESCAN) {
		Read reader(comms);
		uint64_t index = reader.readUInt64();
		timmers[index].cancel();

		timmers.erase(timmers.begin() + index);
	}
}

void Server::requestDel() {
	Read reader(comms);
	uint64_t threatIndex = reader.readUInt64();
	std::u16string threatPath = viruses->get(threatIndex);
	Write writer(comms);
	bool success = false;
	WaitForSingleObject(mutex, INFINITE);
	if (DeleteFile((wchar_t*)threatPath.c_str())) {
		viruses->remove(threatIndex);
		viruses->save();
		success = true;
	}
	else if (GetLastError() == 2) {
		viruses->remove(threatIndex);
		viruses->save();
		success = true;
	}
	ReleaseMutex(mutex);
	writer.writeUInt8((uint8_t)success);
}

void Server::startScan() {
	Read reader(comms);
	std::u16string path = reader.readU16String();
	std::u16string reportPath = reader.readU16String();
	hReportAddress = INVALID_HANDLE_VALUE;
	hReportAddress = CreateFile((LPCWSTR)reportPath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	scanner.startScan(path, hReportAddress);
}

void Server::timedScan() {
	Read reader(comms);
	std::u16string scanPath = reader.readU16String();
	uint32_t hours = reader.readUInt32();
	uint32_t minutes = reader.readUInt32();
	timmers.push_back(Timmer(db, viruses, scanPath, hours, minutes));
	std::thread scheduleThread(&Timmer::start, std::ref(timmers[timmers.size() - 1]));
	scheduleThread.detach();
}

void Server::shutDown() {
	saveMonitors();
	saveTimmers();
	ExitProcess(0);
}

void Server::loadMonitors() {
	std::u16string filePath = u"monitors.gor";
	Read reader(filePath);
	if (!reader.isOpen())
		return;
	std::u16string header = reader.readU16String();
	if (header != u"gorelov") {
		reader.close();
		return;
	}
	uint64_t recordNumber = reader.readUInt64();
	for (size_t i = 0; i < recordNumber; i++) {
		std::u16string scanPath = reader.readU16String();
		monitors.push_back(Monitor(scanPath, db, viruses));
		std::thread monitorThread(&Monitor::start, std::ref(monitors[monitors.size() - 1]));
		monitorThread.detach();
	}
	reader.close();

}

void Server::saveMonitors() {
	std::u16string filePath = u"monitors.gor";
	Write writer(filePath);
	writer.writeU16String(u"gorelov");
	writer.writeUInt64(monitors.size());
	for (auto& el : monitors) {
		writer.writeU16String(el.getPath());
	}
	writer.close();
}

void Server::loadTimmers() {
	std::u16string filePath = u"scanners.gor";
	Read reader(filePath);
	if (!reader.isOpen())
		return;
	std::u16string header = reader.readU16String();
	if (header != u"gorelov") {
		reader.close();
		return;
	}
	uint64_t recordNumber = reader.readUInt64();
	for (size_t i = 0; i < recordNumber; i++) {
		std::u16string scanPath = reader.readU16String();
		uint32_t hours = reader.readUInt32();
		uint32_t minutes = reader.readUInt32();

		timmers.push_back(Timmer(db, viruses, scanPath, hours, minutes));

		std::thread monitorThread(&Timmer::start, std::ref(timmers[timmers.size() - 1]));
		monitorThread.detach();
	}
	reader.close();
}

void Server::saveTimmers() {
	std::u16string filePath = u"scanners.gor";
	Write writer(filePath);
	writer.writeU16String(u"gorelov");
	writer.writeUInt64(timmers.size());
	for (auto& el : timmers) {
		writer.writeU16String(el.getPath());
		writer.writeUInt32(el.getHours());
		writer.writeUInt32(el.getMinutes());
	}
	writer.close();
}

void Server::start() {
	db = std::shared_ptr<DB>(DBLoad::load(u"baza.gor"));
	viruses = std::make_shared<Viruses>(u"viruses.gor");
	viruses->load();
	scanner = Scanner(db, viruses);
	loadMonitors();
	loadTimmers();
	std::thread commsThread(&Server::startReading, this);
	commsThread.join();
}