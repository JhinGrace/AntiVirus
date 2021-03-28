#include "CommsMail.h"

CommsMail::CommsMail(const std::u16string& readPath, const std::u16string& writePath)
{
	this->readPath = readPath;
	this->writePath = writePath;
	SECURITY_DESCRIPTOR sd;
	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, true, NULL, false);
	SECURITY_ATTRIBUTES sa;
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = true;
	readSlot = CreateMailslot((LPCWSTR)readPath.c_str(),
		0, MAILSLOT_WAIT_FOREVER, (LPSECURITY_ATTRIBUTES)&sa);
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
	HRESULT xyi = GetLastError();
	writeSlot = CreateFile((LPCWSTR)writePath.c_str(),
		GENERIC_WRITE,
		FILE_SHARE_READ,
		(LPSECURITY_ATTRIBUTES)NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		(HANDLE)NULL);
	xyi = GetLastError();
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
		xyi = GetLastError();
	}
}

void CommsMail::clear()
{
	DWORD cbMessage, cMessage, cAllMessages, cbRead;

	HRESULT fResult = GetMailslotInfo(readSlot,
		(LPDWORD)NULL,
		&cbMessage,
		&cMessage,
		(LPDWORD)NULL);

	if (cbMessage == MAILSLOT_NO_MESSAGE)
	{
		return;
	}

	cAllMessages = cMessage;

	char buffer[2048];

	while (cMessage != 0)  // retrieve all messages
	{

		fResult = ReadFile(readSlot,
			buffer,
			cbMessage,
			&cbRead,
			NULL);

		cMessage--;
	}
}

void CommsMail::disconnect()
{
	CloseHandle(readSlot);
	CloseHandle(writeSlot);
}