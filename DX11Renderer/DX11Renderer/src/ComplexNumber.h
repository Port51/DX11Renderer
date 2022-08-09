#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class ComplexNumber
	{
	public:
		ComplexNumber();
		ComplexNumber(const float real, const float imaginary);

    public:
        const float GetRealComponent() const;
        const float GetImaginaryComponent() const;

    public:
        ComplexNumber& operator+=(const ComplexNumber& v)
        {
            m_realComponent += v.m_realComponent;
            m_imaginaryComponent += v.m_imaginaryComponent;
            return *this;
        }

        ComplexNumber& operator*=(const ComplexNumber& v)
        {
            // Multiplication of complex numbers
            // P * Q = PrQr - PiQi + [PrQi + PrQi]i
            const float r = m_realComponent * v.m_realComponent - m_imaginaryComponent * v.m_imaginaryComponent;
            const float i = m_realComponent * v.m_imaginaryComponent + m_realComponent * v.m_imaginaryComponent;
            m_realComponent = r;
            m_imaginaryComponent = i;
            return *this;
        }

	private:
		float m_realComponent;
		float m_imaginaryComponent;
	};

    inline ComplexNumber operator+(const ComplexNumber& u, const ComplexNumber& v)
    {
        return ComplexNumber(u.GetRealComponent() + v.GetRealComponent(), u.GetImaginaryComponent() + v.GetImaginaryComponent());
    }

    inline ComplexNumber operator-(const ComplexNumber& u, const ComplexNumber& v)
    {
        return ComplexNumber(u.GetRealComponent() - v.GetRealComponent(), u.GetImaginaryComponent() - v.GetImaginaryComponent());
    }

    inline ComplexNumber operator*(const ComplexNumber& u, const ComplexNumber& v)
    {
        // Multiplication of complex numbers
        // P * Q = PrQr - PiQi + [PrQi + PrQi]i
        return ComplexNumber(u.GetRealComponent() * v.GetRealComponent() - u.GetImaginaryComponent() * v.GetImaginaryComponent(),
            u.GetRealComponent() * v.GetImaginaryComponent() + u.GetRealComponent() * v.GetImaginaryComponent());
    }
}