#pragma once
#include <exception>
#include <string>

class CustomException : public std::exception
{
public:
	CustomException(int line, const char* file) noexcept;
	const char* what() const noexcept override;
	virtual const char* GetType() const noexcept;
	int GetLine() const noexcept;
	const std::string& GetFile() const noexcept;
	std::string GetOriginString() const noexcept;
private:
	int line; // line of exception
	std::string file; // code file
protected:
	// mutable allows setting from const what()
	mutable std::string whatBuffer; // allows return of what() to live past that method
};