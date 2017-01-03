//
// pch.h
//
// Pre-compiled header file.
//
#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>

#include <wrl.h>
#include <shellapi.h>

#include <string>
#include <cstdlib>
#include <cassert>

#include "Core/Types.hpp"
#include "Core/DebugUtils.hpp"
#include "Core/Memory.hpp"
