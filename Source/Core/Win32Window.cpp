#include "pch.h"

#include <cassert>
#include <chrono>
#include <cwchar>
#include <thread>

#include "Core/Win32Window.hpp"
#include "Core/GameApplication.hpp"

HWND Win32Window::hwnd = nullptr;



//---------------------------------------------------------------------------------------
int Win32Window::Run (
	GameApplication * game,
    HINSTANCE hInstance,
    int nCmdShow
) {
	assert(game);

    //-- Open a new console window and redirect std streams to it:
    {
        // Open a new console window
        AllocConsole();

        //-- Associate std input/output with newly opened console window:
        FILE * file0 = freopen("CONIN$", "r", stdin);
        FILE * file1 = freopen("CONOUT$", "w", stdout);
        FILE * file2 = freopen("CONOUT$", "w", stderr);
    }

	//-- Initialize the window class:
	WNDCLASSEX windowClass = { 0 };
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc = WindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	windowClass.lpszClassName = "Win32Window";
	RegisterClassEx(&windowClass);

    //-- Center window:
    RECT windowRect;
	GetClientRect(GetDesktopWindow(), &windowRect);
	long width = game->getWindowWidth();
	long height = game->getWindowHeight();
	windowRect.left = (windowRect.right / 2) - (width / 2);
	windowRect.top = (windowRect.bottom / 2) - (height / 2);

	AdjustWindowRect(&windowRect, WS_OVERLAPPEDWINDOW, FALSE);

	// Create the window and store a handle to it.
	hwnd = CreateWindow (
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
		game            // Store pointer to the D3D12Demo in the user data slot.
                        // We'll retrieve this later within the WindowProc in order
                        // interact with the D3D12Demo instance.
    );

	ShowWindow(hwnd, nCmdShow);

    //-- Timing information:
    static uint32 frameCount(0);
    static float fpsTimer(0.0f);

	// Main sample loop.
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
        // Start frame timer.
        auto timerStart = std::chrono::high_resolution_clock::now();
        // Process any messages in the queue.
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            // Translate virtual-key codes into character messages.
            TranslateMessage(&msg);

            // Dispatches a message to a the registered window procedure.
            DispatchMessage(&msg);
        }

		std::this_thread::sleep_for(std::chrono::milliseconds(16));

        // End frame timer.
        auto timerEnd = std::chrono::high_resolution_clock::now();
        ++frameCount;

        auto timeDelta = 
            std::chrono::duration<double, std::milli>(timerEnd - timerStart).count();
        fpsTimer += (float)timeDelta;

        //-- Update window title only after so many milliseconds:
        if (fpsTimer > 400.0f) {
			float msPerFrame = fpsTimer / float(frameCount);
            float fps = float(frameCount) / fpsTimer * 1000.0f;
            char buffer[256];
			sprintf(buffer, "%s - %.1f fps (%.2f ms)", 
				game->getWindowTitle(), fps, msPerFrame);
            SetWindowText(hwnd, buffer);

            // Reset timing info.
            fpsTimer = 0.0f;
            frameCount = 0;
        }
	}

	// Return this part of the WM_QUIT message to Windows.
	return static_cast<char>(msg.wParam);
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
			// Save the GameApplication ptr passed into CreateWindow and store it in the
			// window's user data field so we can retrieve it later.
			LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA,
                reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		}
		return 0;

	case WM_KEYDOWN:
        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
        }
		else if (game)
		{
			LOG_INFO("KeyDown");
			LOG_WARNING("KeyDown");
			LOG_ERROR("KeyDown");
			game->onKeyDown(static_cast<UINT8>(wParam));
		}
		return 0;

	case WM_KEYUP:
		if (game)
		{
			game->onKeyUp(static_cast<UINT8>(wParam));
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
