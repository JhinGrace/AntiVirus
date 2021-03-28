#include "Timmer.h"
#include "Windows.h"
#include <thread>

Timmer::Timmer(const std::shared_ptr<DB>& db, const std::shared_ptr<Viruses>& viruses,
	const std::u16string path /*= u""*/, uint32_t hours /*= 0*/, uint32_t minutes /*= 0*/) : scanner(db, viruses) {
	scanPath = path;
	this->hours = hours;
	this->minutes = minutes;
}

Timmer& Timmer::operator=(const Timmer& other) {
	scanner = other.scanner;
	hours = other.hours;
	minutes = other.minutes;
	scanPath = other.scanPath;

	return *this;
}

void Timmer::setScanTime(uint32_t hours, uint32_t minutes) {
	this->hours = hours;
	this->minutes = minutes;
}

void Timmer::setScanPath(const std::u16string& path) {
	scanPath = path;
}


void Timmer::start() {
	timeMonitoring();
}

void Timmer::timeMonitoring() {
	SYSTEMTIME now;
	GetLocalTime(&now);
	while (true) {
		if (shouldStop)
			return;
		GetLocalTime(&now);
		if (now.wHour == hours && now.wMinute == minutes) {
			scanner.startScan(scanPath);
			Sleep(1000 * 60);
		}
		Sleep(1000);
	}

}

