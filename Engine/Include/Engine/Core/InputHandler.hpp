//
// InputHandler.hpp
//
#pragma once

#include "Core/Types.hpp"

class InputHandler {
public:
	void keyDown (
		uint8 virtualKey
	);

	void keyUp (
		uint8 virtualKey
	);

private:
	// 32bit Bit Field denoting key status.
	// Bit value of 1 denotes key for specifici bit is currently down.
	uint _keyboardStatus = 0;
};
