#pragma once

#include <Windows.h>

class GameApplication;

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
	static HWND _hWnd;

};
