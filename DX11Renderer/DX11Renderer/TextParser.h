#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std::string_literals;

class TextParser
{
public:
	class ParsedKeyValues
	{
	public:
		std::string key;
		std::vector<std::string> values;
	};

	TextParser(std::string_view filePath)
		: file(std::string(filePath).c_str())
	{
	}

	~TextParser()
	{
		Dispose();
	}

	void Dispose()
	{
		if (fileOpen)
		{
			file.close();
			fileOpen = false;
		}
	}

	bool ReadParsedLine(ParsedKeyValues& result)
	{
		if (!file.is_open())
		{
			return false;
		}

		std::string line;
		if (std::getline(file, line))
		{
			// Clean up line first
			line.erase(std::remove_if(line.begin(), line.end(), IsWhitespace), line.end());

			std::istringstream iss(line);

			// Read property key
			getline(iss, result.key, ',');

			// Read values
			result.values.clear();
			std::string token;
			while (getline(iss, token, ','))
			{
				result.values.push_back(token);
			}

			return true;
		}
		return false;
	}
private:
	bool IsWhitespace(unsigned char c)
	{
		return (c == ' ' || c == '\n' || c == '\r' ||
			c == '\t' || c == '\v' || c == '\f');
	}

private:
	std::ifstream file;
	bool fileOpen;
};