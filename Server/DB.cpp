#include "DB.h"
#include <../PicoSHA2/picosha2.h>
#include <memory>

DB::DB(std::unordered_multimap<uint64_t, Log>&& db)
{
	this->db = std::move(db);
}

DB::DB(DB&& other)
{
	this->db = std::move(other.db);
}

bool DB::Compare(char* address, uint64_t offset, const std::u16string& type, std::u16string& name) {
	uint64_t key;
	memcpy((void*)&key, (void*)address, sizeof(key));
	auto entries = db.equal_range(key);
	for (auto& it = entries.first; it != entries.second; it++) {
		if (it->second.type == type) {
			if (offset >= it->second.offsetStart && offset <= it->second.offsetEnd) {
				if (it->second.length == 8) {
					name = it->second.name;
					return true;
				}
				std::vector<char> bytes(address + 8, address + it->second.length);
				std::string hash_hex_str;
				picosha2::hash256_hex_string(bytes, hash_hex_str);
				if (it->second.sha256 == hash_hex_str) {
					name = it->second.name;
					return true;
				}
			}
		}
	}
	return false;
}


DB& DB::operator=(DB&& other)
{
	this->db = std::move(other.db);
	return *this;
}

