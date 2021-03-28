#pragma once
#include "Liba.h"
#include <string>

class CommsMail : public Comms {
public:
	CommsMail(const std::u16string& readPath, const std::u16string& writePath);
	~CommsMail();
	virtual void connect() override;
	virtual HANDLE readHandle() override { return readSlot; }
	virtual HANDLE writeHandle() override { return writeSlot; }
	virtual void clear() override;
	virtual void disconnect() override;

private:
	HANDLE readSlot = INVALID_HANDLE_VALUE;
	HANDLE writeSlot = INVALID_HANDLE_VALUE;
	std::u16string readPath;
	std::u16string writePath;
};