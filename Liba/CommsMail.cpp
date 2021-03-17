#include "CommsMail.h"

CommsMail::CommsMail(const std::u16string& readPath, const std::u16string& writePath)
{
	this->readPath = readPath;
	this->writePath = writePath;
	readSlot = CreateMailslot((LPCWSTR)readPath.c_str(),
		0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)NULL);
}

CommsMail::~CommsMail()
{
	if (readSlot != INVALID_HANDLE_VALUE)
		CloseHandle(readSlot);
	if (writeSlot != INVALID_HANDLE_VALUE)
		CloseHandle(writeSlot);
}

void CommsMail::connect()
{
	writeSlot = CreateFile((LPCWSTR)writePath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);

	while (writeSlot == INVALID_HANDLE_VALUE)
	{
		Sleep(100);
		writeSlot = CreateFile((LPCWSTR)writePath.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			(LPSECURITY_ATTRIBUTES)NULL,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			(HANDLE)NULL);
	}
}