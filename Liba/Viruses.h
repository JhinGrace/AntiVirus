#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <Windows.h>

extern HANDLE mutex;

class Viruses {
public:
	Viruses(const std::u16string& path);
	~Viruses();
	void load();
	void save();
	void add(const std::u16string& path);
	void add(uint64_t threatIndex, const std::u16string& threatPath);
	void remove(uint64_t index);
	inline std::u16string get(size_t index) { return viruses[index]; }
	inline size_t size() { return viruses.size(); }
	void log();

private:
	std::vector<std::u16string> viruses;
	std::u16string path;
	HANDLE mutex;
};
