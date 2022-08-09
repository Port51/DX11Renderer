#pragma once
#include "pch.h"
#include "Complex.h"

namespace gfx
{
	Complex::Complex()
		: m_realComponent(0.f), m_imaginaryComponent(0.f)
	{}

	Complex::Complex(const float real, const float imaginary)
		: m_realComponent(real), m_imaginaryComponent(imaginary)
	{}

	const float Complex::GetRealComponent() const
	{
		return m_realComponent;
	}

	const float Complex::GetImaginaryComponent() const
	{
		return m_imaginaryComponent;
	}
}