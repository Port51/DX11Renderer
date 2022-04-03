#include "CorePch.h"
#include "CommonCoreHeader.h"

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

LPCWSTR CharArrayToLPCWSTR(const char * input)
{
	int lenA = lstrlenA(input);
	int lenW = MultiByteToWideChar(CP_ACP, 0, input, lenA, NULL, 0);

	wchar_t* output = nullptr;
	if (lenW > 0)
	{
		output = new wchar_t[lenW];
		MultiByteToWideChar(CP_ACP, 0, input, lenA, output, lenW);
	}
	return output;
}

#ifdef UNICODE
#define LPC_STRING_TYPE LPCWSTR
LPCWSTR SafeLPC(const char* input)
{
	return CharArrayToLPCWSTR(input);
}
LPCWSTR SafeLPC(const std::string& input)
{
	return CharArrayToLPCWSTR(input.c_str());
}
#else
#define LPC_STRING_TYPE LPCSTR
LPCSTR SafeLPC(const char* input)
{
	return input;
}
LPCSTR SafeLPC(const std::string& input)
{
	return input.c_str();
}
#endif
