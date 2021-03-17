#pragma once
#include <string>
#include <memory>
#include <Windows.h>

class CommsMail;

enum class CMDCODE : uint8_t
{
	SERVERSHUTDOWN = 0, CLIENTSHUTDOWN = 1, SCAN = 2
};

class Comms
{
public:
	static std::shared_ptr<CommsMail> Mailslots(const std::u16string& readPath, const std::u16string& writePath);
	virtual void connect() = 0;
	virtual HANDLE readHandle() = 0;
	virtual HANDLE writeHandle() = 0;
};
