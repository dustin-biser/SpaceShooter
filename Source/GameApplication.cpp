#include "pch.h"

#include "GameApplication.hpp"


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
const char * GameApplication::getWindowTitle() const
{
	return _windowTitle;
}

//---------------------------------------------------------------------------------------
uint GameApplication::getWindowWidth() const
{
	return _windowWidth;
}

//---------------------------------------------------------------------------------------
uint GameApplication::getWindowHeight() const
{
	return _windowHeight;
}

//---------------------------------------------------------------------------------------
void GameApplication::onKeyDown (
	uint8 key
) {

}

//---------------------------------------------------------------------------------------
void GameApplication::onKeyUp (
	uint8 key
) {

}

