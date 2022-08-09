#pragma once
#include "pch.h"
#include "ComplexNumber.h"

namespace gfx
{
	ComplexNumber::ComplexNumber()
		: m_realComponent(0.f), m_imaginaryComponent(0.f)
	{}

	ComplexNumber::ComplexNumber(const float real, const float imaginary)
		: m_realComponent(real), m_imaginaryComponent(imaginary)
	{}

	const float ComplexNumber::GetRealComponent() const
	{
		return m_realComponent;
	}

	const float ComplexNumber::GetImaginaryComponent() const
	{
		return m_imaginaryComponent;
	}
}