#pragma once
#include <stdexcept>
#include <string>
#include <sstream>

namespace gfx
{

	class GfxException : public std::exception
	{
	public:
		GfxException(const size_t line, const char* file, const HRESULT hr);
		GfxException(const size_t line, const char* file, const std::string msg = "");
		GfxException(const size_t line, const char* file, const char* msg = "");
		const char* what() const override;
	private:
		const std::string HrToMessageString(const HRESULT hr) const;
		const std::string HrToStringCode(const HRESULT hr) const;
		const std::string GetExceptionString(int line, const char* file, const HRESULT hr) const;
	private:
		const size_t m_line;
		const char* m_file;
		std::string m_msg;
	};

}

#define THROW_IF_FAILED(hr) if (FAILED(hr)) { throw GfxException(__LINE__, __FILE__, hr); }
#define THROW_ALWAYS(hr) throw GfxException(__LINE__, __FILE__, hr)
#define THROW(msg) throw GfxException(__LINE__, __FILE__, msg)