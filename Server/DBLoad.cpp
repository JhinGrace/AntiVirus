#include "DBLoad.h"
#include <Liba.h>
#include <Read.h>


DB* DBLoad::load(const std::u16string& path)
{
	Read reader(path);

	std::u16string header = reader.readU16String();
	if (header != std::u16string(u"gorelov"))
		return new DB();

	uint64_t rowCount = reader.readUInt64();

	std::unordered_multimap<uint64_t, Log> db;
	Log log;
	for (int i = 0; i < rowCount; i++)
	{
		log.name = reader.readU16String().c_str();
		log.type = reader.readU16String().c_str();
		log.length = reader.readUInt64();

		uint64_t sigStart = reader.readUInt64();
		std::reverse((uint8_t*)&sigStart, ((uint8_t*)&sigStart) + 8);

		log.sigStart = sigStart;
		log.offsetStart = reader.readUInt64();
		log.offsetEnd = reader.readUInt64();
		log.sha256 = reader.readASCIIString().c_str();

		db.insert({ log.sigStart, log });
	}

	reader.close();

	return new DB(std::move(db));
}
