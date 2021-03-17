#include "Write.h"

Write::Write(const std::u16string& path)
{
	handle = CreateFile((wchar_t*)path.c_str(), GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		handle = CreateFile((wchar_t*)path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
}

Write::Write(const std::shared_ptr<Comms>& commType)
{
	handle = commType->writeHandle();
}

void Write::close()
{
	CloseHandle(handle);
}

void Write::writeInt8(int8_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeInt16(int16_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeInt32(int32_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeInt64(int64_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeUInt8(uint8_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeUInt16(uint16_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeUInt32(uint32_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeUInt64(uint64_t value)
{
	DWORD bytesWritten;
	WriteFile(handle, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Write::writeU16String(const std::u16string& value)
{
	writeUInt32((uint32_t)value.size());
	DWORD bytesWritten;
	WriteFile(handle, (const void*)value.c_str(), value.size() * sizeof(char16_t), &bytesWritten, NULL);
}

void Write::writeASCIIString(const std::string& value)
{
	writeUInt32((uint32_t)value.size());
	DWORD bytesWritten;
	WriteFile(handle, (const void*)value.c_str(), value.size() * sizeof(char), &bytesWritten, NULL);
}