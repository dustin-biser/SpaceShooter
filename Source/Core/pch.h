//
// pch.h
//
// Pre-compiled header file.
//
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

#include <wrl.h>
#include <shellapi.h>

#include <string>
#include <cstdlib>

#include "Core/Common.hpp"
#include "Core/DebugUtils.hpp"
