#pragma once
#include <memory>
#include "Target.h"
#include "DB.h"

class Engine {
public:
	Engine(const std::shared_ptr<DB>& db);
	Engine();
	~Engine() = default;
	Engine& operator=(const Engine& other);
	bool scan(const Target& target, std::u16string& virusName);

private:
	bool scanFile(const Target& target, std::u16string& virusName);
	bool scanMemory(const Target& target, std::u16string& virusName);
	bool scanZipEntry(const Target& target, std::u16string& virusName);
	void updateBufferFromFile(std::ifstream& file);
	void updateBufferFromZipEntry(zip_file* file);

private:
	std::shared_ptr<DB> db;
	const size_t bufferSize = 1024 * 1024;
	std::vector<char> buffer;
	const size_t minSigLength = 8;
	const size_t maxSigLength = 1024;
};
