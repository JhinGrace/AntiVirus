#include <fstream>
#include "Engine.h"


Engine::Engine(const std::shared_ptr<DB>& db)
{
	this->db = db;

	// 1 MB buffer
	buffer.resize(bufferSize, 0);
}

Engine::Engine()
{
}

Engine& Engine::operator=(const Engine& other)
{
	buffer = other.buffer;
	db = other.db;

	return *this;
}

bool Engine::scan(const Target& target, std::u16string& virusName)
{
	if (target.objtype == OBJTYPE::DIRENTRY)
	{
		if (scanFile(target, virusName))
			return true;
	}
	else if (target.objtype == OBJTYPE::MEMORY)
	{
		if (scanMemory(target, virusName))
			return true;
	}
	else if (target.objtype == OBJTYPE::ZIPENTRY)
	{
		if (scanZipEntry(target, virusName))
			return true;
	}
	return false;
}

bool Engine::scanFile(const Target& target, std::u16string& virusName)
{
	std::ifstream fileStream((wchar_t*)target.filePath.c_str(), std::ios::binary | std::ios::ate);

	size_t fileSize = fileStream.tellg();

	fileStream.seekg(0);

	fileStream.read(buffer.data(), bufferSize);

	size_t offset = 0;

	for (size_t i = 0; i < fileSize / bufferSize; i++)
	{
		for (size_t j = 0; j < bufferSize - maxSigLength; j++)
		{
			if (db->Compare(buffer.data() + j, offset, target.fileType, virusName))
			{
				fileStream.close();
				return true;
			}

			offset++;
		}
		updateBufferFromFile(fileStream);
	}

	// check last chunk of data
	for (size_t i = 0; i < bufferSize - minSigLength; i++)
	{
		if (db->Compare(buffer.data() + i, offset, target.fileType, virusName))
		{
			fileStream.close();
			return true;
		}

		offset++;
	}
	fileStream.close();
	return false;
}

bool Engine::scanMemory(const Target& target, std::u16string& virusName)
{
	for (size_t i = 0; i < target.size - minSigLength; i++)
	{
		if (db->Compare(buffer.data() + i, i, target.fileType, virusName))
			return true;
	}

	return false;
}

bool Engine::scanZipEntry(const Target& target, std::u16string& virusName)
{
	zip_file* file = zip_fopen_index(target.archive, target.index, 0);

	const char* name = zip_get_name(target.archive, target.index, 0);

	struct zip_stat st;
	zip_stat_init(&st);
	zip_stat(target.archive, name, 0, &st);


	zip_fread(file, buffer.data(), bufferSize);

	size_t offset = 0;

	for (size_t i = 0; i < st.size / bufferSize; i++)
	{
		for (size_t j = 0; j < bufferSize - maxSigLength; j++)
		{
			if (db->Compare(buffer.data() + j, offset, target.fileType, virusName))
			{
				zip_fclose(file);
				return true;
			}

			offset++;
		}
		updateBufferFromZipEntry(file);
	}

	// check last chunk of data
	for (size_t i = 0; i < bufferSize - minSigLength; i++)
	{
		if (db->Compare(buffer.data() + i, offset, target.fileType, virusName))
		{
			zip_fclose(file);
			return true;
		}
		offset++;
	}
	zip_fclose(file);
	return false;
}

void Engine::updateBufferFromFile(std::ifstream& file)
{
	std::copy(buffer.end() - maxSigLength, buffer.end(), buffer.begin());
	memset(buffer.data() + maxSigLength, 0, bufferSize - maxSigLength);
	file.read(buffer.data() + maxSigLength, bufferSize - maxSigLength);
}

void Engine::updateBufferFromZipEntry(zip_file* file)
{
	std::copy(buffer.end() - maxSigLength, buffer.end(), buffer.begin());
	memset(buffer.data() + maxSigLength, 0, bufferSize - maxSigLength);
	zip_fread(file, buffer.data() + maxSigLength, bufferSize - maxSigLength);
}
