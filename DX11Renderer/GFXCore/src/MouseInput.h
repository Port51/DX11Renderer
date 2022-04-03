#pragma once
#include "MouseEvent.h"
#include <queue>

/*
* GFXCore:
* Shared logic that is used for all Windows graphics projects
*/

class MouseInput
{
public:
	void OnMouseMove(int x, int y);
	void OnLeftButtonDown(int x, int y);
	void OnLeftButtonUp(int x, int y);
	void OnRightButtonDown(int x, int y);
	void OnRightButtonUp(int x, int y);
	void OnMiddleButtonDown(int x, int y);
	void OnMiddleButtonUp(int x, int y);
	void OnWheelUp(int x, int y);
	void OnWheelDown(int x, int y);

	bool IsLeftButtonDown();
	bool IsMiddleButtonDown();
	bool IsRightButtonDown();

	int GetMousePosX();
	int GetMousePosY();
	MousePoint GetMousePos();

	bool EventBufferIsEmpty();
	MouseEvent ReadEvent();

private:
	std::queue<MouseEvent> m_eventBuffer;
	bool m_leftIsDown = false;
	bool m_rightIsDown = false;
	bool m_mbuttonDown = false;
	int m_x = 0;
	int m_y = 0;
};