#include "CorePch.h"
#include "TextParser.h"
#include <algorithm>

namespace gfxcore
{
	// Returns as int
	const int TextParser::ParsedKeyValues::ReadInt(int index) const
	{
		return std::stoi(values[index]);
	}

	// Returns as int, using std::move()
	const int TextParser::ParsedKeyValues::MoveInt(int index) const
	{
		return std::stoi(std::move(values[index]));
	}

	// Returns as float
	const float TextParser::ParsedKeyValues::ReadFloat(int index) const
	{
		return std::stof(values[index]);
	}

	// Returns as float, using std::move()
	const float TextParser::ParsedKeyValues::MoveFloat(int index) const
	{
		return std::stof(std::move(values[index]));
	}

	TextParser::TextParser(std::string_view filePath)
		: file(std::string(filePath).c_str())
	{}

	TextParser::~TextParser()
	{
		Release();
	}

	void TextParser::Release()
	{
		if (fileOpen)
		{
			file.close();
			fileOpen = false;
		}
	}

	bool TextParser::ReadParsedLine(ParsedKeyValues & result, char delimiter)
	{
		if (!file.is_open())
		{
			return false;
		}

		std::string line;
		if (std::getline(file, line))
		{
			// Clean up line first
			line.erase(std::remove_if(line.begin(), line.end(), &IsWhitespace), line.end());

			std::istringstream iss(std::move(line));

			// Read property key
			std::getline(iss, result.key, delimiter);

			// Read values
			result.values.clear();
			std::string token;
			while (std::getline(iss, token, delimiter))
			{
				result.values.push_back(std::move(token));
			}

			return true;
		}
		return false;
	}

	bool TextParser::IsWhitespace(unsigned char c)
	{
		return (c == ' ' || c == '\n' || c == '\r' ||
			c == '\t' || c == '\v' || c == '\f');
	}

}