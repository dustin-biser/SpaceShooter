//
// Main.cpp
//

#include "pch.h"
#include "Win32Application.hpp"
#include "GameApplication.hpp"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	GameApplication game = GameApplication(1024, 768, "Space Shooter");
	return Win32Application::Run(&game, hInstance, nCmdShow);
}
