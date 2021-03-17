#include "Liba.h"
#include "CommsMail.h"

std::shared_ptr<CommsMail> Comms::Mailslots(const std::u16string& readPath, const std::u16string& writePath)
{
	return std::make_shared<CommsMail>(readPath, writePath);
}
