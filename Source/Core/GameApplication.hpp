//
// GameApplication.hpp
//

#pragma once

#include "NumericTypes.hpp"

#include "InputHandler.hpp"

class GameApplication {
public:
	GameApplication (
		uint windowWidth,
		uint windowHeight,
		const char * windowTitle
	);

	const char * getWindowTitle() const;

	uint getWindowWidth() const;

	uint getWindowHeight() const;

	void initialze();

	void keyDown (
		uint8 virtualKey
	);

	void keyUp (
		uint8 virtualKey
	);


private:
	uint _windowWidth;
	uint _windowHeight;
	const char * _windowTitle;

	InputHandler _inputHandler;
};
