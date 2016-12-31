#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>

#include <Engine/Engine.h>

class Win32Window {
public:
	static int Run (
		GameApplication * game,
        HINSTANCE hInstance,
        int nCmdShow
    );

	static HWND GetHwnd ();

protected:
	static LRESULT CALLBACK WindowProc (
        HWND hWnd,
        UINT message,
        WPARAM wParam,
        LPARAM lParam
    );

private:
	// Handle to window
	static HWND _hWindow;
};
