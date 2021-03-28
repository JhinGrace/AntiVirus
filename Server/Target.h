#pragma once
#include <stdint.h>
#include <string>
#include <zip.h>

enum class OBJTYPE {NONE = 0, DIRENTRY, ZIPENTRY, MEMORY};

struct Target
{
	OBJTYPE objtype = OBJTYPE::NONE;
	std::u16string fileType = u"";
	std::u16string filePath = u"";
	zip_t* archive = nullptr;
	zip_int64_t index = 0;
	uint8_t* address = nullptr;
	size_t size = 0;
};