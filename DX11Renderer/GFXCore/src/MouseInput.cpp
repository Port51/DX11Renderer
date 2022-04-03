#include "CorePch.h"
#include "MouseInput.h"

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

void MouseInput::OnMouseMove(int x, int y)
{
	m_x = x;
	m_y = y;
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::MouseMove, x, y));
}

void MouseInput::OnLeftButtonDown(int x, int y)
{
	m_leftIsDown = true;
	MouseEvent evt(MouseEvent::EventType::LeftButtonDown, x, y);
	m_eventBuffer.push(evt);
}

void MouseInput::OnLeftButtonUp(int x, int y)
{
	m_leftIsDown = false;
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::LeftButtonUp, x, y));
}

void MouseInput::OnRightButtonDown(int x, int y)
{
	m_rightIsDown = true;
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::RightButtonDown, x, y));
}

void MouseInput::OnRightButtonUp(int x, int y)
{
	m_rightIsDown = false;
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::RightButtonUp, x, y));
}

void MouseInput::OnMiddleButtonDown(int x, int y)
{
	m_mbuttonDown = true;
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::MiddleButtonDown, x, y));
}

void MouseInput::OnMiddleButtonUp(int x, int y)
{
	m_mbuttonDown = false;
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::MiddleButtonUp, x, y));
}

void MouseInput::OnWheelUp(int x, int y)
{
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::WheelUp, x, y));
}

void MouseInput::OnWheelDown(int x, int y)
{
	m_eventBuffer.push(MouseEvent(MouseEvent::EventType::WheelDown, x, y));
}

bool MouseInput::IsLeftButtonDown()
{
	return m_leftIsDown;
}

bool MouseInput::IsMiddleButtonDown()
{
	return m_mbuttonDown;
}

bool MouseInput::IsRightButtonDown()
{
	return m_rightIsDown;
}

int MouseInput::GetMousePosX()
{
	return m_x;
}

int MouseInput::GetMousePosY()
{
	return m_y;
}

MousePoint MouseInput::GetMousePos()
{
	return{ m_x, m_y };
}

bool MouseInput::EventBufferIsEmpty()
{
	return m_eventBuffer.empty();
}

MouseEvent MouseInput::ReadEvent()
{
	if (m_eventBuffer.empty())
	{
		return MouseEvent();
	}
	else
	{
		MouseEvent evt = m_eventBuffer.front();
		m_eventBuffer.pop();
		return evt;
	}
}