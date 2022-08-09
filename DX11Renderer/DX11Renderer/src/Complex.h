#pragma once
#include "CommonHeader.h"

namespace gfx
{
	class Complex
	{
	public:
		Complex();
		Complex(const float real, const float imaginary);

    public:
        const float GetRealComponent() const;
        const float GetImaginaryComponent() const;

    public:
        Complex& operator+=(const Complex& v)
        {
            m_realComponent += v.m_realComponent;
            m_imaginaryComponent += v.m_imaginaryComponent;
            return *this;
        }

        Complex& operator*=(const Complex& v)
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

    inline Complex operator+(const Complex& u, const Complex& v)
    {
        return Complex(u.GetRealComponent() + v.GetRealComponent(), u.GetImaginaryComponent() + v.GetImaginaryComponent());
    }

    inline Complex operator-(const Complex& u, const Complex& v)
    {
        return Complex(u.GetRealComponent() - v.GetRealComponent(), u.GetImaginaryComponent() - v.GetImaginaryComponent());
    }

    inline Complex operator*(const Complex& u, const Complex& v)
    {
        // Multiplication of complex numbers
        // P * Q = PrQr - PiQi + [PrQi + PrQi]i
        return Complex(u.GetRealComponent() * v.GetRealComponent() - u.GetImaginaryComponent() * v.GetImaginaryComponent(),
            u.GetRealComponent() * v.GetImaginaryComponent() + u.GetRealComponent() * v.GetImaginaryComponent());
    }
}