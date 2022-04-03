#pragma once

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

struct MousePoint
{
	int x;
	int y;
};

class MouseEvent
{
public:
	enum EventType
	{
		Undefined,
		MouseMove,
		LeftButtonDown,
		LeftButtonUp,
		RightButtonDown,
		RightButtonUp,
		MiddleButtonDown,
		MiddleButtonUp,
		WheelUp,
		WheelDown,
	};
public:
	MouseEvent();
	MouseEvent(const EventType type, const int x, const int y);
	bool IsValid() const;
	EventType GetType() const;
	MousePoint GetPos() const;
	int GetPosX() const;
	int GetPosY() const;
private:
	EventType m_type;
	int m_x;
	int m_y;
};