//
// Main.cpp
//

#include "Win32Window.hpp"

int WINAPI WinMain (
	HINSTANCE hInstance,
	HINSTANCE, 
	LPSTR, 
	int nCmdShow
) {
	// Stack allocate.
	GameApplication game = GameApplication(1024, 768, "Space Shooter");
	return Win32Window::Run(&game, hInstance, nCmdShow);
}
