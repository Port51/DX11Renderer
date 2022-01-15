#include "pch.h"
#include "CustomException.h"
#include <sstream>


CustomException::CustomException(int line, const char* file)
	:
	line(line),
	file(file)
{}

const char* CustomException::what() const
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* CustomException::GetType() const
{
	return "Chili Exception";
}

int CustomException::GetLine() const
{
	return line;
}

const std::string& CustomException::GetFile() const
{
	return file;
}

std::string CustomException::GetOriginString() const
{
	std::ostringstream oss;
	oss << "[File] " << file << std::endl
		<< "[Line] " << line;
	return oss.str();
}