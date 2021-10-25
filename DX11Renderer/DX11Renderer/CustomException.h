#pragma once
#include <exception>
#include <string>

class CustomException : public std::exception
{
public:
	CustomException(int line, const char* file);
	const char* what() const override;
	virtual const char* GetType() const;
	int GetLine() const;
	const std::string& GetFile() const;
	std::string GetOriginString() const;
private:
	int line; // line of exception
	std::string file; // code file
protected:
	// mutable allows setting from const what()
	mutable std::string whatBuffer; // allows return of what() to live past that method
};