#include "Viruses.h"
#include "Read.h"
#include "Write.h"
#include <algorithm>

Viruses::Viruses(const std::u16string& path) {
	this->path = path;
	mutex = OpenMutex(SYNCHRONIZE, FALSE, L"Mutex");
}

Viruses::~Viruses() {
	CloseHandle(mutex);
}

void Viruses::load() {
	WaitForSingleObject(mutex, INFINITE);
	viruses.resize(0);
	Read reader(path);
	if (!reader.isOpen()) {
		ReleaseMutex(mutex);
		return;
	}
	std::u16string header = reader.readU16String();
	if (header != u"gorelov") {
		reader.close();
		ReleaseMutex(mutex);
		return;
	}
	uint64_t logCount = reader.readUInt64();
	for (size_t i = 0; i < logCount; i++) {
		viruses.push_back(reader.readU16String());
	}
	reader.close();
	ReleaseMutex(mutex);

}

void Viruses::save() {
	WaitForSingleObject(mutex, INFINITE);
	Write writer(path);
	writer.writeU16String(u"gorelov");
	writer.writeUInt64((uint64_t)viruses.size());
	for (size_t i = 0; i < viruses.size(); i++) {
		writer.writeU16String(viruses[i]);
	}
	writer.close();
	ReleaseMutex(mutex);
}

void Viruses::add(const std::u16string& threatPath) {
	std::u16string path = threatPath;
	std::replace(path.begin(), path.end(), u'\\', u'/');
	if (std::find(viruses.begin(), viruses.end(), path) == viruses.end())
		viruses.push_back(path);
}

void Viruses::add(uint64_t threatIndex, const std::u16string& threatPath) {
	viruses.insert(viruses.begin() + threatIndex, threatPath);
}

void Viruses::remove(uint64_t index) {
	viruses.erase(viruses.begin() + index);
}

void Viruses::log() {
	for (std::u16string el : viruses) {
		std::wcout << (wchar_t*)el.c_str() << std::endl;
	}
}

