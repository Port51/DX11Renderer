#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>

using namespace std::string_literals;

namespace gfx
{
	class TextParser
	{
	public:
		class ParsedKeyValues
		{
		public:
			std::string key;
			std::vector<std::string> values;
		};

		TextParser(std::string_view filePath);

		virtual ~TextParser();

		void Dispose();

		bool ReadParsedLine(ParsedKeyValues& result);
	private:
		static bool IsWhitespace(unsigned char c);

	private:
		std::ifstream file;
		bool fileOpen;
	};
}