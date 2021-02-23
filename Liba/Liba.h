#pragma once
#include <Windows.h>
#include <stdint.h>
#include <string>
#include <vector>

enum class TYPECODE : int8_t
{
	UINT8, UINT16, UINT32, UINT64,
	INT8 = 0, INT16, INT32, INT64,
	FLOAT32, FLOAT64,
	U16STRING, U16CHAR,
	STRUCT,

	// arrays
	AUINT8, AUINT16, AUINT32, AUINT64,
	AINT8, AINT16, AINT32, AINT64,
	AFLOAT32, AFLOAT64
};
enum class CMDCODE : int8_t
{
	TEST = 0
};

class Liba {
public:
	//create, connect and disconnect
	static HANDLE CreateSlot(const std::u16string& name);
	static HANDLE ConnectToSlot(const std::u16string& name);
	static void Disconnect(HANDLE hSlot);

	//necessary checks
	static bool HasNewData(HANDLE hSlot);
	static bool IsInvalid(HANDLE hSlot);
	inline static void Wait(HANDLE hSlot) { while (!HasNewData(hSlot)) {} }

	//ints
	static void WriteInt8(HANDLE hSlot, int8_t value);
	static void WriteInt16(HANDLE hSlot, int16_t value);
	static void WriteInt32(HANDLE hSlot, int32_t value);
	static void WriteInt64(HANDLE hSlot, int64_t value);
	static int8_t  ReadInt8(HANDLE hSlot);
	static int16_t ReadInt16(HANDLE hSlot);
	static int32_t ReadInt32(HANDLE hSlot);
	static int64_t ReadInt64(HANDLE hSlot);

	//string and char
	static void WriteU16String(HANDLE hSlot, const std::u16string& value);
	static void WriteU16Char(HANDLE hSlot, char16_t value);
	static std::u16string ReadU16String(HANDLE hSlot);
	static char16_t ReadU16Char(HANDLE hSlot);
};