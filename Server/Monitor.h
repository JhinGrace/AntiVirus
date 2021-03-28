#pragma once

#include "Scanner.h"
#include "DB.h"

class Monitor {
public:
	Monitor(const std::u16string& path, const std::shared_ptr<DB>& db, const std::shared_ptr<Viruses>& viruses);
	Monitor& operator=(const Monitor& other);
	Monitor();
	~Monitor() = default;
	void start();
	void stop();
	inline std::u16string getPath() { return path; }

private:
	void run();

private:
	Scanner scanner;
	std::u16string path;
	HANDLE changeHandle = INVALID_HANDLE_VALUE;
	bool shouldStop = false;
	bool shouldPause = false;
};
