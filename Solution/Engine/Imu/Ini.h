#pragma once
#include <map>
#include <string>

class Ini
{
public:
	Ini(const char* filePath);
	std::string getValue(const char* section, const char* key);
private:
	void raise(const std::string& string);
	std::map<std::string, std::map<std::string, std::string>> content;
};
