//
// GameApplication.cpp
//

#include "pch.h"

#include "Core/GameApplication.hpp"

#include "Graphics/D3D12Renderer.hpp"


//---------------------------------------------------------------------------------------
GameApplication::GameApplication (
	uint windowWidth,
	uint windowHeight,
	const char * windowTitle
) 
	: _windowWidth(windowWidth),
	  _windowHeight(windowHeight),
	  _windowTitle(windowTitle)
{

}

//---------------------------------------------------------------------------------------
const char * GameApplication::getWindowTitle () const
{
	return _windowTitle;
}

//---------------------------------------------------------------------------------------
uint GameApplication::getWindowWidth () const
{
	return _windowWidth;
}

//---------------------------------------------------------------------------------------
uint GameApplication::getWindowHeight () const
{
	return _windowHeight;
}

//---------------------------------------------------------------------------------------
void GameApplication::initialze (
	HWND hWindow
) {
	// Allocate instance for D3D12Renderer.
	_renderer = std::make_shared<D3D12Renderer> ();
	_renderer->initialize (hWindow);
}

//---------------------------------------------------------------------------------------
void GameApplication::keyDown (
	uint8 virtualKey
) {
	_inputHandler.keyDown(virtualKey);
}

//---------------------------------------------------------------------------------------
void GameApplication::keyUp (
	uint8 virtualKey
) {
	_inputHandler.keyUp(virtualKey);
}

//---------------------------------------------------------------------------------------
void GameApplication::update ()
{
	_renderer->render ();
	_renderer->present ();
}

