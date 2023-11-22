#include <fstream>
#include <stdexcept>
#include "ini.h"

Ini::Ini(const char* filePath)
{
	std::ifstream file(filePath);

	if (!file)
	{
		this->raise(std::string("File not found: ") + filePath);
	}

	std::string line;
	std::string section;
	while (getline(file, line))
	{
		if (line[0] == '\0' || line[0] == ';') continue;
		if (line[0] == '[')
		{
			const size_t endIndex = line.find("]");
			if (endIndex == std::string::npos)
			{
				this->raise("Invalid section: " + line);
			}
			section = line.substr(1, endIndex - 1);
			this->content[section] = std::map<std::string, std::string>();
		} else
		{
			const size_t equalIndex = line.find(" = ");
			if (equalIndex == std::string::npos)
			{
				this->raise("Invalid key/value: " + line);
			}
			this->content[section][line.substr(0, equalIndex)] = line.substr(equalIndex + 3, line.size());
		}
	}
}

std::string Ini::getValue(const char* section, const char* key)
{
	if (!this->content.count(section))
	{
		this->raise(std::string("Section not found: ") + section);
	}

	if (!this->content[section].count(key))
	{
		this->raise(std::string("Key not found: ") + key);
	}

	return this->content[section][key];
}

void Ini::raise(const std::string& string)
{
	throw std::runtime_error(std::string("[Ini] ") + string);
}