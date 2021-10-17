#include "CustomException.h"
#include <sstream>


CustomException::CustomException(int line, const char* file) noexcept
	:
	line(line),
	file(file)
{}

const char* CustomException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* CustomException::GetType() const noexcept
{
	return "Chili Exception";
}

int CustomException::GetLine() const noexcept
{
	return line;
}

const std::string& CustomException::GetFile() const noexcept
{
	return file;
}

std::string CustomException::GetOriginString() const noexcept
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}