//
// InputHandler.cpp
//
#include "pch.h"
#include "InputHandler.hpp"

#if defined(WIN32)
	#include "WinUser.h"
#endif

// Bit field values for key status: 
// Bit == 1: key is down.
// Bit == 0: key is up.
#define KEY_A_BIT (1 << 0)
#define KEY_S_BIT (1 << 1)
#define KEY_D_BIT (1 << 2)
#define KEY_W_BIT (1 << 3)
#define KEY_SPACE_BIT (1 << 4)


//---------------------------------------------------------------------------------------
void InputHandler::keyDown (
	uint8 virtualKey
) {
	switch (virtualKey) {
		case 'A': _keyboardStatus |= KEY_A_BIT; break;
		case 'S': _keyboardStatus |= KEY_S_BIT; break;
		case 'D': _keyboardStatus |= KEY_D_BIT; break;
		case 'W': _keyboardStatus |= KEY_W_BIT; break;
		case VK_SPACE: _keyboardStatus |= KEY_SPACE_BIT; break;
		default: break;
	}

	LOG_INFO("%c Key Pressed", static_cast<char>(virtualKey));
}

//---------------------------------------------------------------------------------------
void InputHandler::keyUp (
	uint8 virtualKey
) {
	switch (virtualKey) {
		case 'A': _keyboardStatus &= ~KEY_A_BIT; break;
		case 'S': _keyboardStatus &= ~KEY_S_BIT; break;
		case 'D': _keyboardStatus &= ~KEY_D_BIT; break;
		case 'W': _keyboardStatus &= ~KEY_W_BIT; break;
		case VK_SPACE: _keyboardStatus &= ~KEY_SPACE_BIT; break;
		default: break;
	}

	LOG_INFO("%c Key Up", static_cast<char>(virtualKey));
}
