#include <filesystem>
#include <iostream>
#include <thread>
#include <Liba.h>
#include <sstream>
#include <Write.h>
#include <Read.h>
#include "Scanner.h"
#include <zip.h>

#define MZHEADER 0x5a4d
#define ZIPHEADER 0x04034b50

Scanner::Scanner(const std::shared_ptr<DB>& db,
	const std::shared_ptr<Viruses>& viruses) : engine(db) {
	this->db = db;
	this->viruses = viruses;
}

Scanner::Scanner() {

}

Scanner& Scanner::operator=(const Scanner& other) {
	engine = other.engine;
	db = other.db;
	viruses = other.viruses;
	shouldStop = other.shouldStop;

	return *this;
}

void Scanner::startScan(const std::u16string& path, HANDLE hReportAddress, bool async) {
	shouldStop = false;
	stopped = false;

	if (async) {
		std::thread scanThread(&Scanner::scanWithReport, this, path, hReportAddress);
		scanThread.detach();
	}
	else {
		scanWithReport(path, hReportAddress);
	}
}

void Scanner::startScan(const std::u16string& path, bool async) {
	shouldStop = false;
	stopped = false;

	if (async) {
		std::thread scanThread(&Scanner::scan, this, path);
		scanThread.detach();
	}
	else {
		scan(path);
	}
}

void Scanner::stopScan() {
	shouldStop = true;
}

void Scanner::scanWithReport(const std::u16string& path, HANDLE hReportAddress) {
	viruses->load();
	if (std::filesystem::is_directory(path))
		scanDirectory(path, hReportAddress);
	else
		scanFile(path, hReportAddress);
	viruses->save();
	CloseHandle(hReportAddress);
	stopped = true;
}

void Scanner::scan(const std::u16string& path) {
	viruses->load();
	if (std::filesystem::is_directory(path)) {
		scanDirectory(path);
	}
	else {
		scanFile(path);
	}
	viruses->save();
	stopped = true;
}

void Scanner::scanDirectory(const std::u16string& path, HANDLE hReportAddress) {
	using std::filesystem::recursive_directory_iterator;
	using fp = bool (*)(const std::filesystem::path&);
	uint64_t numberOfFiles = std::count_if(recursive_directory_iterator(path),
		recursive_directory_iterator{}, (fp)std::filesystem::is_regular_file);
	Write writer(hReportAddress);
	writer.writeUInt64(numberOfFiles);
	std::u16string virusName;
	for (const auto& dirEntry : recursive_directory_iterator(path)) {
		if (shouldStop) {
			return;
		}
		bool safe = true;
		if (std::filesystem::is_directory(dirEntry))
			continue;
		std::u16string path = dirEntry.path().u16string();
		std::ifstream fileStream((wchar_t*)path.c_str(), std::ios::binary);
		uint16_t mzHeader = 0;
		uint32_t zipHeader = 0;
		fileStream.read((char*)&mzHeader, sizeof(uint16_t));
		fileStream.seekg(0);
		fileStream.read((char*)&zipHeader, sizeof(uint32_t));
		fileStream.close();
		if (mzHeader == MZHEADER) {
			Target target;
			target.objtype = OBJTYPE::DIRENTRY;
			target.filePath = path;
			target.fileType = u"PE";
			if (engine.scan(target, virusName))
				safe = false;
		}
		else if (zipHeader == ZIPHEADER) {
			Target target;
			target.objtype = OBJTYPE::DIRENTRY;
			target.filePath = path;
			target.fileType = u"ZIP";
			if (scanZip(target, virusName))
				safe = false;
		}
		writer.writeU16String(path);
		writer.writeUInt8((uint8_t)safe);
		if (!safe) {
			writer.writeU16String(virusName);
			viruses->add(path);
		}
	}
}

void Scanner::scanDirectory(const std::u16string& path)
{
	using std::filesystem::recursive_directory_iterator;
	using fp = bool (*)(const std::filesystem::path&);
	uint64_t numberOfFiles = std::count_if(recursive_directory_iterator(path),
		recursive_directory_iterator{}, (fp)std::filesystem::is_regular_file);
	std::u16string virusName;
	for (const auto& dirEntry : recursive_directory_iterator(path)) {
		if (shouldStop)
			return;
		bool safe = true;
		if (std::filesystem::is_directory(dirEntry))
			continue;
		std::u16string path = dirEntry.path().u16string();
		std::ifstream fileStream((wchar_t*)path.c_str(), std::ios::binary);
		uint16_t mzHeader = 0;
		uint32_t zipHeader = 0;
		fileStream.read((char*)&mzHeader, sizeof(uint16_t));
		fileStream.seekg(0);
		fileStream.read((char*)&zipHeader, sizeof(uint32_t));
		fileStream.close();
		if (mzHeader == MZHEADER) {
			Target target;
			target.objtype = OBJTYPE::DIRENTRY;
			target.filePath = path;
			target.fileType = u"PE";

			if (engine.scan(target, virusName))
				safe = false;
		}
		else if (zipHeader == ZIPHEADER) {
			Target target;
			target.objtype = OBJTYPE::DIRENTRY;
			target.filePath = path;
			target.fileType = u"ZIP";
			if (scanZip(target, virusName))
				safe = false;
		}
		if (!safe) {
			viruses->add(path);
		}
	}
}

void Scanner::scanFile(const std::u16string& path, HANDLE hReportAddress) {
	if (shouldStop)
		return;
	Write writer(hReportAddress);
	uint64_t numberOfFiles = 1;
	writer.writeUInt64(numberOfFiles);
	std::u16string virusName;
	bool safe = true;
	std::ifstream fileStream((wchar_t*)path.c_str(), std::ios::binary);
	uint16_t mzHeader = 0;
	uint32_t zipHeader = 0;
	fileStream.read((char*)&mzHeader, sizeof(uint16_t));
	fileStream.seekg(0);
	fileStream.read((char*)&zipHeader, sizeof(uint32_t));
	fileStream.close();
	if (mzHeader == MZHEADER) {
		Target target;
		target.objtype = OBJTYPE::DIRENTRY;
		target.filePath = path;
		target.fileType = u"PE";

		if (engine.scan(target, virusName))
			safe = false;
	}
	else if (zipHeader == ZIPHEADER) {
		Target target;
		target.objtype = OBJTYPE::DIRENTRY;
		target.filePath = path;
		target.fileType = u"ZIP";


		if (scanZip(target, virusName))
			safe = false;
	}
	writer.writeU16String(path);
	writer.writeUInt8((uint8_t)safe);
	if (!safe) {
		writer.writeU16String(virusName);
		viruses->add(path);
	}
}

void Scanner::scanFile(const std::u16string& path)
{
	if (shouldStop)
		return;
	std::u16string virusName;
	bool safe = true;
	std::ifstream fileStream((wchar_t*)path.c_str(), std::ios::binary);
	uint16_t mzHeader = 0;
	uint32_t zipHeader = 0;
	fileStream.read((char*)&mzHeader, sizeof(uint16_t));
	fileStream.seekg(0);
	fileStream.read((char*)&zipHeader, sizeof(uint32_t));
	fileStream.close();
	if (mzHeader == MZHEADER) {
		Target target;
		target.objtype = OBJTYPE::DIRENTRY;
		target.filePath = path;
		target.fileType = u"PE";
		if (engine.scan(target, virusName))
			safe = false;
	}
	else if (zipHeader == ZIPHEADER) {
		Target target;
		target.objtype = OBJTYPE::DIRENTRY;
		target.filePath = path;
		target.fileType = u"ZIP";
		if (scanZip(target, virusName))
			safe = false;
	}
	if (!safe) {
		viruses->add(path);
	}
}

bool Scanner::scanZip(const Target& target, std::u16string& virusName) {
	if (shouldStop)
		return false;
	if (engine.scan(target, virusName))
		return true;
	zip_source_t* src = nullptr;
	zip_t* archive = nullptr;
	zip_error_t error;
	zip_error_init(&error);
	// create archive source
	if (target.objtype == OBJTYPE::DIRENTRY) {
		if ((src = zip_source_win32w_create((wchar_t*)target.filePath.c_str(), 0, -1, &error)) == NULL) {
			fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
			zip_error_fini(&error);
			return NULL;
		}
	}
	else if (target.objtype == OBJTYPE::ZIPENTRY) {
		if ((src = zip_source_zip(target.archive, target.archive, target.index, 0, 0, 0)) == NULL) {
			fprintf(stderr, "can't create source: %s\n", zip_error_strerror(&error));
			zip_error_fini(&error);
			return NULL;
		}
	}
	else
		return false;
	/* open zip archive from source */
	if ((archive = zip_open_from_source(src, 0, &error)) == NULL) {
		fprintf(stderr, "can't open zip from source: %s\n", zip_error_strerror(&error));
		zip_source_free(src);
		zip_error_fini(&error);
		return NULL;
	}
	zip_error_fini(&error);
	zip_int64_t num_entries = zip_get_num_entries(archive, /*flags=*/0);
	for (zip_int64_t i = 0; i < num_entries; i++) {
		if (shouldStop) {
			zip_close(archive);
			return false;
		}
		zip_file* file = zip_fopen_index(archive, i, 0);
		uint16_t mzHeader = 0;
		uint32_t zipHeader = 0;
		zip_fread(file, (char*)&mzHeader, sizeof(uint16_t));
		zip_fseek(file, 0, 0);
		zip_fread(file, (char*)&zipHeader, sizeof(uint32_t));
		zip_fseek(file, 0, 0);
		zip_fclose(file);
		if (mzHeader == MZHEADER) {
			Target target;
			target.objtype = OBJTYPE::ZIPENTRY;
			target.archive = archive;
			target.index = i;
			target.fileType = u"PE";
			if (engine.scan(target, virusName)) {
				zip_close(archive);
				return true;
			}
		}
		else if (zipHeader == ZIPHEADER) {
			Target target;
			target.objtype = OBJTYPE::ZIPENTRY;
			target.archive = archive;
			target.index = i;
			target.fileType = u"ZIP";
			if (scanZip(target, virusName)) {
				zip_close(archive);
				return true;
			}
		}
	}
	zip_close(archive);
	return false;
}
