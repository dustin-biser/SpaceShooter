//
// Main.cpp
//

#include "pch.h"

#include "Core/Win32Window.hpp"
#include "Core/GameApplication.hpp"

int WINAPI WinMain (
	HINSTANCE hInstance,
	HINSTANCE, 
	LPSTR, 
	int nCmdShow
) {
	GameApplication game = GameApplication(1024, 768, "Space Shooter");
	return Win32Window::Run(&game, hInstance, nCmdShow);
}
