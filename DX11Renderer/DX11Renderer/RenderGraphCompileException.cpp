#include "RenderGraphCompileException.h"
#include <sstream>


namespace Rgph
{
	RenderGraphCompileException::RenderGraphCompileException(std::string message, int line, const char* file) noexcept
		:
		CustomException(line, file),
		message(std::move(message))
	{}

	const char* RenderGraphCompileException::what() const noexcept
	{
		std::ostringstream oss;
		oss << CustomException::what() << std::endl;
		oss << "[message]" << std::endl;
		oss << message;
		whatBuffer = oss.str();
		return whatBuffer.c_str();
	}

	const char* RenderGraphCompileException::GetType() const noexcept
	{
		return "Render Graph Compile Exception";
	}


	const std::string& RenderGraphCompileException::GetMessage() const noexcept
	{
		return message;
	}
}