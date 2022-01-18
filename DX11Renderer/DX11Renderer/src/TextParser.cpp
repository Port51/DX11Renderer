#include "pch.h"
#include "TextParser.h"

namespace gfx
{
	TextParser::TextParser(std::string_view filePath)
		: file(std::string(filePath).c_str())
	{}
	TextParser::~TextParser()
	{
		Dispose();
	}
	void TextParser::Dispose()
	{
		if (fileOpen)
		{
			file.close();
			fileOpen = false;
		}
	}
	bool TextParser::ReadParsedLine(ParsedKeyValues & result)
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
	bool TextParser::IsWhitespace(unsigned char c)
	{
		return (c == ' ' || c == '\n' || c == '\r' ||
			c == '\t' || c == '\v' || c == '\f');
	}
}