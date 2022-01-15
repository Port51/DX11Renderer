#pragma once
#include <system_error>
#include <string>
#include <sstream>

inline std::string HrToMessageString(HRESULT hr)
{
	return std::system_category().message(hr);
}

inline std::string HrToStringCode(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

inline std::string GetExceptionString(int line, const char* file, HRESULT hr)
{
	std::ostringstream oss;
	oss << "[Error Code] " << HrToStringCode(hr) << " (" << std::to_string(hr) << ")" << std::endl << std::endl
		<< "[File] " << std::string(file) << std::endl
		<< "[Line] " << std::to_string(line) << std::endl << std::endl
		<< "[Description]" << HrToMessageString(hr);
	return oss.str();
}

class HrException : public std::runtime_error
{
public:
	HrException(int line, const char* file, HRESULT hr) : std::runtime_error(GetExceptionString(line, file, hr)), m_hr(hr) {}
	HRESULT Error() const { return m_hr; }
private:
	const HRESULT m_hr;
};

#define THROW_IF_FAILED(hrcall) if( FAILED( hr = (hrcall) ) ) throw HrException(__LINE__, __FILE__, hr)
#define THROW_ALWAYS(hrcall) throw HrException(__LINE__, __FILE__, hr)