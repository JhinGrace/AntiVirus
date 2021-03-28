#pragma once
#include "DB.h"

class DBLoad {
public:
	static DB* load(const std::u16string& path);
};