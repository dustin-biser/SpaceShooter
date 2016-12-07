#include "pch.h"

#include <cassert>
#include <chrono>
#include <cwchar>
#include <thread>

#include "Core/Win32Window.hpp"
#include "Core/GameApplication.hpp"

HWND Win32Window::_hWindow = nullptr;



//---------------------------------------------------------------------------------------
int Win32Window::Run (
	GameApplication * game,
    HINSTANCE hInstance,
    int nCmdShow
) {
	ASSERT(game);

	//-- Initialize the window class:
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "Win32Window";
	::RegisterClassEx(&windowClass);

    //-- Center window:
    RECT windowRect;
	GetClientRect(GetDesktopWindow(), &windowRect);
	long width = game->getWindowWidth();
	long height = game->getWindowHeight();
	windowRect.left = (windowRect.right / 2) - (width / 2);
	windowRect.top = (windowRect.bottom / 2) - (height / 2);

	::AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	_hWindow = ::CreateWindow (
		windowClass.lpszClassName,
		game->getWindowTitle(),
		WS_OVERLAPPEDWINDOW,
		windowRect.left,
		windowRect.top,
        width,
        height,
		nullptr,		// We have no parent window.
		nullptr,		// We aren't using menus.
		hInstance,
		game            // Store pointer to GameApplication instance in the user data slot.
                        // We'll retrieve this later within the WindowProc in order
                        // to forward messages to the GameApplication.
    );

	game->initialze (_hWindow);

	::ShowWindow (_hWindow, nCmdShow);

    //-- Timing information:
    static uint32 frameCount(0);
    static float fpsTimer(0.f);

	// Main sample loop.
	MSG msg = {0};
	while (msg.message != WM_QUIT)
	{
        // Start frame timer.
        auto timerStart = std::chrono::high_resolution_clock::now();
        // Process any messages in the queue.
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // Translate virtual-key codes into character messages.
            ::TranslateMessage(&msg);

            // Dispatches message to the registered window procedure.
            ::DispatchMessage(&msg);
        }

		game->update ();

        // End frame timer.
        auto timerEnd = std::chrono::high_resolution_clock::now();
        ++frameCount;

        auto timeDelta = std::chrono::duration<double, std::milli>(timerEnd - timerStart).count();
        fpsTimer += static_cast<float>(timeDelta);

        //-- Update window title only after so many milliseconds:
        if (fpsTimer > 400.0f) {
			float msPerFrame = fpsTimer / float(frameCount);
            float fps = float(frameCount) / fpsTimer * 1000.0f;
            char buffer[256];
			sprintf(buffer, "%s - %.1f fps (%.2f ms)", 
				game->getWindowTitle(), fps, msPerFrame);
            ::SetWindowText(_hWindow, buffer);

            // Reset timing info.
            fpsTimer = 0.0f;
            frameCount = 0;
        }
	}

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
}


//---------------------------------------------------------------------------------------
HWND Win32Window::GetHwnd()
{
	return _hWindow;
}

//---------------------------------------------------------------------------------------
// Main message handler.
LRESULT CALLBACK Win32Window::WindowProc (
    HWND hWnd,
    UINT message,
    WPARAM wParam,
    LPARAM lParam
) {
    // Retrieve a pointer to the GameApplication instance held by the user data field of
	// our window instance.
	GameApplication * game =
		reinterpret_cast<GameApplication*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (message)
	{
	case WM_CREATE:
		{
			// Save the GameApplication pointer passed into CreateWindow and store it in
			// the window's user data field so we can retrieve it later.
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

	case WM_LBUTTONDOWN:
	{
		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hWnd, &p);
		LOG_INFO("Cursor: (%d,%d)", p.x, p.y);
		return 0;
	}

	case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
		else if (game)
		{
			game->keyDown(static_cast<uint8>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (game)
		{
			game->keyUp(static_cast<uint8>(wParam));
		}
		return 0;


	case WM_PAINT:
        ValidateRect(hWnd, NULL);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	// Handle all other messages.
	return DefWindowProc(hWnd, message, wParam, lParam);
}
