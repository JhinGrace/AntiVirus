#pragma once
#include "Liba.h"
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>

class Read {
public:
	Read(const std::u16string& path);
	Read(const std::shared_ptr<Comms>& commType);
	Read(HANDLE handle) { this->handle = handle; }
	~Read() = default;
	void close();
	inline bool isOpen() { return handle != INVALID_HANDLE_VALUE; }
	int8_t  readInt8();
	int16_t readInt16();
	int32_t readInt32();
	int64_t readInt64();
	uint8_t readUInt8();
	uint16_t readUInt16();
	uint32_t readUInt32();
	uint64_t readUInt64();
	std::u16string readU16String();
	std::string readASCIIString();

private:
	HANDLE handle;
};