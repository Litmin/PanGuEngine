#include "pch.h"
#include "Input.h"

using namespace std;
using namespace DirectX;

std::array<KeyState, (size_t)KeyCode::COUNT> Input::m_KeyStates = {};
XMINT2 Input::m_MousePosition = XMINT2(0, 0);

void Input::Update()
{
	for (KeyState& keyState : m_KeyStates)
	{
		if (((int)keyState & (int)KeyState::KeyUp) != 0)
		{
			keyState = KeyState::None;
		}

		if (((int)keyState & (int)KeyState::KeyDown) != 0)
		{
			keyState = KeyState::KeyHold;
		}
	}
}

bool Input::GetKey(KeyCode keyCode)
{
	return ((int)m_KeyStates[(int)keyCode] & (int)KeyState::KeyHold) != 0;
}

bool Input::GetKeyUp(KeyCode keyCode)
{
	return ((int)m_KeyStates[(int)keyCode] & (int)KeyState::KeyUp) != 0;
}

bool Input::GetKeyDown(KeyCode keyCode)
{
	return ((int)m_KeyStates[(int)keyCode] & (int)KeyState::KeyDown) != 0;
}

XMINT2 Input::GetMousePosition()
{
	return m_MousePosition;
}

void Input::OnKeyUp(int key)
{
	switch (key)
	{
	case VK_ESCAPE:
		m_KeyStates[(int)KeyCode::Escape] = KeyState::KeyUp;
		break;

	case VK_UP:
		m_KeyStates[(int)KeyCode::UpArrow] = KeyState::KeyUp;
		break;

	case VK_DOWN:
		m_KeyStates[(int)KeyCode::DownArrow] = KeyState::KeyUp;
		break;

	case VK_LEFT:
		m_KeyStates[(int)KeyCode::LeftArrow] = KeyState::KeyUp;
		break;

	case VK_RIGHT:
		m_KeyStates[(int)KeyCode::RightArrow] = KeyState::KeyUp;
		break;
	
	case 'W':
		m_KeyStates[(int)KeyCode::W] = KeyState::KeyUp;
		break;

	case 'A':
		m_KeyStates[(int)KeyCode::A] = KeyState::KeyUp;
		break;

	case 'S':
		m_KeyStates[(int)KeyCode::S] = KeyState::KeyUp;
		break;

	case 'D':
		m_KeyStates[(int)KeyCode::D] = KeyState::KeyUp;
		break;

	case 'Q':
		m_KeyStates[(int)KeyCode::Q] = KeyState::KeyUp;
		break;

	case 'E':
		m_KeyStates[(int)KeyCode::E] = KeyState::KeyUp;
		break;
	}
}

void Input::OnKeyDown(int key)
{
	switch (key)
	{
	case VK_ESCAPE:
		m_KeyStates[(int)KeyCode::Escape] = KeyState::KeyDown;
		break;

	case VK_UP:
		m_KeyStates[(int)KeyCode::UpArrow] = KeyState::KeyDown;
		break;

	case VK_DOWN:
		m_KeyStates[(int)KeyCode::DownArrow] = KeyState::KeyDown;
		break;

	case VK_LEFT:
		m_KeyStates[(int)KeyCode::LeftArrow] = KeyState::KeyDown;
		break;

	case VK_RIGHT:
		m_KeyStates[(int)KeyCode::RightArrow] = KeyState::KeyDown;
		break;

	case 'W':
		m_KeyStates[(int)KeyCode::W] = KeyState::KeyDown;
		break;

	case 'A':
		m_KeyStates[(int)KeyCode::A] = KeyState::KeyDown;
		break;

	case 'S':
		m_KeyStates[(int)KeyCode::S] = KeyState::KeyDown;
		break;

	case 'D':
		m_KeyStates[(int)KeyCode::D] = KeyState::KeyDown;
		break;

	case 'Q':
		m_KeyStates[(int)KeyCode::Q] = KeyState::KeyDown;
		break;

	case 'E':
		m_KeyStates[(int)KeyCode::E] = KeyState::KeyDown;
		break;
	}
}

void Input::OnMouseMove(int x, int y)
{
	m_MousePosition = XMINT2(x, y);
}
