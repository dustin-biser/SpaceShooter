#pragma once

#include "NumericTypes.hpp"

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

	void onKeyDown(uint8 key);

	void onKeyUp(uint8 key);

private:
	uint _windowWidth;
	uint _windowHeight;
	const char * _windowTitle;
};
