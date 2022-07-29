#pragma once
#include <vector>
#include <string>
#include <string_view>
#include <fstream>
#include <sstream>

using namespace std::string_literals;

namespace gfxcore
{
	class TextParser
	{
	public:
		class ParsedKeyValues
		{
		public:
			std::string key;
			std::vector<std::string> values;
			std::string line;
		public:
			const int ReadInt(int index) const;
			const int MoveInt(int index) const;
			const float ReadFloat(int index) const;
			const float MoveFloat(int index) const;
			const std::string ReadString(int index) const;
			const std::string MoveString(int index) const;
		};

		TextParser(std::string_view filePath);

		virtual ~TextParser();

		void Release();

		const bool ReadParsedLine(ParsedKeyValues& result, const char delimiter = ',');
	private:
		static bool IsWhitespace(unsigned char c);

	private:
		std::ifstream file;
		bool fileOpen;
	};
}