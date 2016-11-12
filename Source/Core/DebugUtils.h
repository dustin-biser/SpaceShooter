//
// DebugUtils.h
//
#pragma once

#include <debugapi.h>

// Logs information string to Output Window.
#if defined(_DEBUG)
#define LOG_INFO(str) \
	OutputDebugStringW(L"Info: " str "\n")
#else
#define LOG_INFO(str) 
#endif
	

// Logs warning string to Output Window.
#if defined(_DEBUG)
#define LOG_WARNING(str) \
	OutputDebugStringW(L"Warning: " str "\n")
#else
#define LOG_WARNING(str) 
#endif


// Logs error string to Output Window.
#if defined(_DEBUG)
#define LOG_ERROR(str) \
	OutputDebugStringW(L"Error: " str "\n")
#else
#define LOG_ERROR(str) 
#endif

