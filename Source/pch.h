/*
 * pch.h
 *
 * Pre-compiled header file.
 */

#pragma once

#ifdef WIN32
    // Allow use of freopen() without compilation warnings/errors.
    // For use with custom allocated console window.
    #define _CRT_SECURE_NO_WARNINGS
    #include <cstdio>
#endif


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers.
#endif

#include <windows.h>

/* Add these later if we use DirectX

	#include <d3d12.h>
	#include <dxgi1_4.h>
	#include <D3Dcompiler.h>
	#include <DirectXMath.h>
	#include "d3dx12.h"
*/

#include <string>
#include <wrl.h>
#include <shellapi.h>
#include <cstdlib>

#include "NumericTypes.hpp"
