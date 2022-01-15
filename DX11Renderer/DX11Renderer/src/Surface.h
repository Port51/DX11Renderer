/******************************************************************************************
*	Chili DirectX Framework Version 16.10.01											  *
*	Surface.h																			  *
*	Copyright 2016 PlanetChili <http://www.planetchili.net>								  *
*																						  *
*	This file is part of The Chili DirectX Framework.									  *
*																						  *
*	The Chili DirectX Framework is free software: you can redistribute it and/or modify	  *
*	it under the terms of the GNU General Public License as published by				  *
*	the Free Software Foundation, either version 3 of the License, or					  *
*	(at your option) any later version.													  *
*																						  *
*	The Chili DirectX Framework is distributed in the hope that it will be useful,		  *
*	but WITHOUT ANY WARRANTY; without even the implied warranty of						  *
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the						  *
*	GNU General Public License for more details.										  *
*																						  *
*	You should have received a copy of the GNU General Public License					  *
*	along with The Chili DirectX Framework.  If not, see <http://www.gnu.org/licenses/>.  *
******************************************************************************************/
#pragma once
#include "WindowsInclude.h"
#include "CustomException.h"
#include <string>
#include <assert.h>
#include <memory>


class Surface
{
public:
	class Color
	{
	public:
		unsigned int dword;
	public:
		constexpr Color() : dword()
		{}
		constexpr Color(const Color& col)
			:
			dword(col.dword)
		{}
		constexpr Color(unsigned int dw)
			:
			dword(dw)
		{}
		constexpr Color(unsigned char x, unsigned char r, unsigned char g, unsigned char b)
			:
			dword((x << 24u) | (r << 16u) | (g << 8u) | b)
		{}
		constexpr Color(unsigned char r, unsigned char g, unsigned char b)
			:
			dword((r << 16u) | (g << 8u) | b)
		{}
		constexpr Color(Color col, unsigned char x)
			:
			Color((x << 24u) | col.dword)
		{}
		Color& operator =(Color color)
		{
			dword = color.dword;
			return *this;
		}
		constexpr unsigned char GetX() const
		{
			return dword >> 24u;
		}
		constexpr unsigned char GetA() const
		{
			return GetX();
		}
		constexpr unsigned char GetR() const
		{
			return (dword >> 16u) & 0xFFu;
		}
		constexpr unsigned char GetG() const
		{
			return (dword >> 8u) & 0xFFu;
		}
		constexpr unsigned char GetB() const
		{
			return dword & 0xFFu;
		}
		void SetX(unsigned char x)
		{
			dword = (dword & 0xFFFFFFu) | (x << 24u);
		}
		void SetA(unsigned char a)
		{
			SetX(a);
		}
		void SetR(unsigned char r)
		{
			dword = (dword & 0xFF00FFFFu) | (r << 16u);
		}
		void SetG(unsigned char g)
		{
			dword = (dword & 0xFFFF00FFu) | (g << 8u);
		}
		void SetB(unsigned char b)
		{
			dword = (dword & 0xFFFFFF00u) | b;
		}
	};
public:
	class Exception : public CustomException
	{
	public:
		Exception(int line, const char* file, std::string note);
		const char* what() const override;
		const char* GetType() const override;
		const std::string& GetNote() const;
	private:
		std::string note;
	};
public:
	Surface(unsigned int width, unsigned int height);
	Surface(Surface&& source);
	Surface(Surface&) = delete;
	Surface& operator=(Surface&& donor);
	Surface& operator=(const Surface&) = delete;
	~Surface();
	void Clear(Color fillValue);
	void PutPixel(unsigned int x, unsigned int y, Color c);
	Color GetPixel(unsigned int x, unsigned int y) const;
	unsigned int GetWidth() const;
	unsigned int GetHeight() const;
	Color* GetBufferPtr();
	const Color* GetBufferPtr() const;
	const Color* GetBufferPtrConst() const;
	static Surface FromFile(const std::string& name);
	void Save(const std::string& filename) const;
	void Copy(const Surface& src);
private:
	Surface(unsigned int width, unsigned int height, std::unique_ptr<Color[]> pBufferParam);
private:
	std::unique_ptr<Color[]> pBuffer;
	unsigned int width;
	unsigned int height;
};