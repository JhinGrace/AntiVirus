#include "Read.h"


Read::Read(const std::u16string& path)
{
	handle = CreateFile((wchar_t*)path.c_str(),
		GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
}

Read::Read(const std::shared_ptr<Comms>& commType)
{
	handle = commType->readHandle();
}

void Read::close()
{
	CloseHandle(handle);
}

int8_t Read::readInt8()
{
	DWORD bytesRead;
	int8_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

int16_t Read::readInt16()
{
	DWORD bytesRead;
	int16_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

int32_t Read::readInt32()
{
	DWORD bytesRead;
	int32_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

int64_t Read::readInt64()
{
	DWORD bytesRead;
	int64_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

uint8_t Read::readUInt8()
{
	DWORD bytesRead;
	uint8_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

uint16_t Read::readUInt16()
{
	DWORD bytesRead;
	uint16_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

uint32_t Read::readUInt32()
{
	DWORD bytesRead;
	uint32_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

uint64_t Read::readUInt64()
{
	DWORD bytesRead;
	uint64_t result;
	ReadFile(handle, (void*)&result, sizeof(result), &bytesRead, NULL);

	return result;
}

std::u16string Read::readU16String()
{
	DWORD bytesRead;
	uint32_t size = readUInt32();
	char16_t result[1024];
	ReadFile(handle, result, size * sizeof(char16_t), &bytesRead, NULL);

	result[size] = u'\0';
	return std::move(std::u16string(result));
}

std::string Read::readASCIIString()
{
	DWORD bytesRead;
	uint32_t size = readUInt32();
	char result[1024];
	ReadFile(handle, result, size * sizeof(char), &bytesRead, NULL);

	result[size] = '\0';
	return std::move(std::string(result));
}