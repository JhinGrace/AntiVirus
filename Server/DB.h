#pragma once
#include <unordered_map>
#include <string>

struct Log
{
	std::u16string name, type;
	uint64_t length, sigStart, offsetStart, offsetEnd;
	std::string sha256;
};

class DB
{
public:
	DB() = default;
	DB(std::unordered_multimap<uint64_t, Log>&& db);
	DB(DB&& other);
	DB& operator=(DB&& other);
	bool Compare(char* address, uint64_t offset, const std::u16string& type, std::u16string& name);

private:
	std::unordered_multimap<uint64_t, Log> db;
};