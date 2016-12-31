//
// GameApplication.hpp
//

#pragma once

#include <memory>

#include <windef.h>

#include "Core/Types.hpp"
#include "Core/InputHandler.hpp"

class IRenderer;

class GameApplication {
public:
	GameApplication (
		uint windowWidth,
		uint windowHeight,
		const char * windowTitle
	);

	const char * getWindowTitle() const;

	uint getWindowWidth () const;

	uint getWindowHeight () const;

	void initialze (
		HWND hWindow
	);

	void keyDown (
		uint8 virtualKey
	);

	void keyUp (
		uint8 virtualKey
	);
	
	void update ();


private:
	uint _windowWidth;
	uint _windowHeight;
	const char * _windowTitle;

	InputHandler _inputHandler;
	std::shared_ptr<IRenderer> _renderer;
};
