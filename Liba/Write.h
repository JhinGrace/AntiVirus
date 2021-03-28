#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include "Liba.h"
#include <memory.h>

class Write {
public:
	Write(const std::u16string& path);
	Write(const std::shared_ptr<Comms>& commType);
	Write(HANDLE handle) { this->handle = handle; }
	~Write() = default;
	void close();
	void writeInt8(int8_t value);
	void writeInt16(int16_t value);
	void writeInt32(int32_t value);
	void writeInt64(int64_t value);
	void writeUInt8(uint8_t value);
	void writeUInt16(uint16_t value);
	void writeUInt32(uint32_t value);
	void writeUInt64(uint64_t value);
	void writeU16String(const std::u16string& value);
	void writeASCIIString(const std::string& value);

private:
	HANDLE handle;
};