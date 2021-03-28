#pragma once
#include <string>
#include <memory>
#include <fstream>
#include <Windows.h>
#include "DB.h"
#include "Target.h"
#include "Engine.h"
#include "Viruses.h"

class Scanner {
public:
	Scanner(const std::shared_ptr<DB>& db,
		const std::shared_ptr<Viruses>& viruses);
	Scanner();
	~Scanner() = default;
	Scanner& operator=(const Scanner& other);
	void startScan(const std::u16string& path, HANDLE hReportAddress, bool async = true);
	void startScan(const std::u16string& path, bool async = false);
	void stopScan();
	inline bool scanStopped() { return stopped; }

private:
	void scanWithReport(const std::u16string& path, HANDLE hReportAddress);
	void scan(const std::u16string& path);
	void scanDirectory(const std::u16string& path, HANDLE hReportAddress);
	void scanFile(const std::u16string& path, HANDLE hReportAddress);
	void scanDirectory(const std::u16string& path);
	void scanFile(const std::u16string& path);
	bool scanZip(const Target& target, std::u16string& virusName);

private:
	std::shared_ptr<DB> db;
	std::shared_ptr<Viruses> viruses;
	Engine engine;
	bool shouldStop = false;
	bool stopped = false;
};