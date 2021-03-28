#include "Monitor.h"

extern HANDLE mutex;

Monitor::Monitor(const std::u16string& path, const std::shared_ptr<DB>& db,
	const std::shared_ptr<Viruses>& viruses)
	: scanner(db, viruses) {
	this->path = path;
}

Monitor::Monitor() {

}

Monitor& Monitor::operator=(const Monitor& other) {
	path = other.path;
	scanner = other.scanner;
	changeHandle = other.changeHandle;
	return *this;
}

void Monitor::start() {
	shouldStop = false;
	shouldPause = false;
	changeHandle = FindFirstChangeNotification((wchar_t*)path.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);
	run();
}

void Monitor::stop() {
	shouldStop = true;
}

void Monitor::run() {
	while (TRUE) {
		WaitForSingleObject(changeHandle, INFINITE);
		while (shouldPause)
			Sleep(1);
		if (shouldStop) {
			CloseHandle(changeHandle);
			return;
		}
		Sleep(100);
		scanner.startScan(path);
		CloseHandle(changeHandle);
		changeHandle = FindFirstChangeNotification((wchar_t*)path.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME);
	}
}

