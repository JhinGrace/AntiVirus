#include "Liba.h"
#include <stdexcept>

HANDLE Liba::CreateSlot(const std::u16string& name)
{
	HANDLE hSlot = CreateMailslot((LPCWSTR)name.c_str(),
		0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)NULL);

	return hSlot;
}

HANDLE Liba::ConnectToSlot(const std::u16string& name)
{
	HANDLE hSlot = CreateFile((LPCWSTR)name.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	return hSlot;
}


void Liba::Disconnect(HANDLE hSlot)
{
	CloseHandle(hSlot);
}

bool Liba::HasNewData(HANDLE hSlot)
{
	DWORD cbMessage, cMessage;
	GetMailslotInfo(hSlot, (LPDWORD)NULL, &cbMessage, &cMessage, (LPDWORD)NULL);

	return cbMessage != MAILSLOT_NO_MESSAGE;
}

bool Liba::IsInvalid(HANDLE hSlot)
{
	return hSlot == INVALID_HANDLE_VALUE;
}

//ints
void Liba::WriteInt8(HANDLE hSlot, int8_t value)
{
	DWORD bytesWritten;
	WriteFile(hSlot, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Liba::WriteInt16(HANDLE hSlot, int16_t value)
{
	DWORD bytesWritten;
	WriteFile(hSlot, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

void Liba::WriteInt32(HANDLE hSlot, int32_t value)
{
	DWORD bytesWritten;
	WriteFile(hSlot, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}


void Liba::WriteInt64(HANDLE hSlot, int64_t value)
{
	DWORD bytesWritten;
	WriteFile(hSlot, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

int8_t Liba::ReadInt8(HANDLE hSlot)
{
	Wait(hSlot);
	DWORD bytesRead;
	int8_t result;
	ReadFile(hSlot, (void*)&result, sizeof(result), &bytesRead, NULL);
	return result;
}

int16_t Liba::ReadInt16(HANDLE hSlot)
{
	Wait(hSlot);
	DWORD bytesRead;
	int16_t result;
	ReadFile(hSlot, (void*)&result, sizeof(result), &bytesRead, NULL);
	return result;
}

int32_t Liba::ReadInt32(HANDLE hSlot)
{
	Wait(hSlot);
	DWORD bytesRead;
	int32_t result;
	ReadFile(hSlot, (void*)&result, sizeof(result), &bytesRead, NULL);
	return result;
}

int64_t Liba::ReadInt64(HANDLE hSlot)
{
	Wait(hSlot);
	DWORD bytesRead;
	int64_t result;
	ReadFile(hSlot, (void*)&result, sizeof(result), &bytesRead, NULL);
	return result;
}
//string and char
void Liba::WriteU16String(HANDLE hSlot, const std::u16string& value)
{
	WriteInt32(hSlot, (int32_t)value.size());
	DWORD bytesWritten;
	WriteFile(hSlot, (const void*)value.c_str(), value.size() * sizeof(char16_t), &bytesWritten, NULL);
}

void Liba::WriteU16Char(HANDLE hSlot, char16_t value)
{
	DWORD bytesWritten;
	WriteFile(hSlot, (const void*)&value, sizeof(value), &bytesWritten, NULL);
}

std::u16string Liba::ReadU16String(HANDLE hSlot)
{
	Wait(hSlot);
	DWORD bytesRead;
	int32_t size = ReadInt32(hSlot);
	char16_t result[1024];
	ReadFile(hSlot, result, size * sizeof(char16_t), &bytesRead, NULL);
	result[size] = u'\0';
	return std::move(std::u16string(result));
}

char16_t Liba::ReadU16Char(HANDLE hSlot)
{
	Wait(hSlot);
	DWORD bytesRead;
	char16_t result;
	ReadFile(hSlot, (void*)&result, sizeof(result), &bytesRead, NULL);
	return result;
}