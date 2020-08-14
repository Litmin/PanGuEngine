#pragma once
#include <array>

enum class KeyCode
{
	None,
	Backspace,
	Tab,
	Clear,
	Return,
	Pause,
	Escape,
	Space,
	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	Keypad0,
	Keypad1,
	Keypad2,
	Keypad3,
	Keypad4,
	Keypad5,
	Keypad6,
	Keypad7,
	Keypad8,
	Keypad9,
	UpArrow,
	DownArrow,
	RightArrow,
	LeftArrow,
	Mouse0,		// Left Mouse Button
	Mouse1,		// Middle Mouse Button
	Mouse2,		// Right Mouse Button
	COUNT
};	 

enum class KeyState
{
	None = 0,
	KeyDown = 1,
	KeyHold = 2,
	KeyUp = 4
};

class Input
{
	friend class Engine;
public:
	static void Update();
	static bool GetKey(KeyCode keyCode);
	static bool GetKeyUp(KeyCode keyCode);
	static bool GetKeyDown(KeyCode keyCode);

private:
	static void OnKeyUp(int key);
	static void OnKeyDown(int key);

private:
	static std::array<KeyState, (size_t)KeyCode::COUNT> m_KeyStates;
};