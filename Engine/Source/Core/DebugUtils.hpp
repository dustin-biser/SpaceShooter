//
// DebugUtils.hpp
//
#pragma once

#include <debugapi.h>
#include <winnt.h>

// Max length for any logged message.
#define LOG_BUFFER_LENGTH 512

// Severity levels for logging:
#define LOG_LEVEL_INFO "Log Info: "
#define LOG_LEVEL_WARNING "Log Warning: "
#define LOG_LEVEL_ERROR "Log Error: "

#if defined(_DEBUG)
#define LOG(message, format, ...) \
	do { \
		char buffer[LOG_BUFFER_LENGTH]; \
		int charsWritten = snprintf (buffer, LOG_BUFFER_LENGTH, message); \
		charsWritten += snprintf (buffer + charsWritten, LOG_BUFFER_LENGTH, format, __VA_ARGS__); \
		snprintf (buffer + charsWritten, LOG_BUFFER_LENGTH, "\n"); \
		OutputDebugString(buffer); \
	} while(0)
#else
#define LOG(levelWString, format, ...)
#endif


#if defined(_DEBUG)
// Logs information string to Output Window.
// @param format - string literal with optional formatting.
#define LOG_INFO(format, ...) LOG(LOG_LEVEL_INFO, format, __VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif
	

#if defined(_DEBUG)
// Logs warning string to Output Window.
// @param format - string literal with optional formatting.
#define LOG_WARNING(format, ...) LOG(LOG_LEVEL_WARNING, format, __VA_ARGS__)
#else
#define LOG_WARNING(format, ...)
#endif


#if defined(_DEBUG)
// Logs error string to Output Window.
// @param format - string literal with optional formatting.
#define LOG_ERROR(format, ...) LOG(LOG_LEVEL_ERROR, format, __VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif


#define toString(x) #x
#define toWideString(x) L#x

#if defined(_DEBUG)
#define CHECK_RESULT(x, str) \
	do { \
		HRESULT result = (x); \
		if ( FAILED(result) ) { \
			LOG_ERROR(toString(str) toString(__FILE__) ":" toString(__LINE__) "\n"); \
			__debugbreak(); \
		} \
	} \
	while(0)
#else
#define CHECK_D3D_RESULT(x) x;
#endif

#if defined(_DEBUG)
#define CHECK_D3D_RESULT(x) CHECK_RESULT(x, "Direct3D error at ");
#else
#define CHECK_D3D_RESULT(x) x;
#endif

#if defined(_DEBUG)
#define CHECK_WIN_RESULT(x) CHECK_RESULT(x, "Windows error at ");
#else
#define CHECK_WIN_RESULT(x) x;
#endif


#if defined(_DEBUG)
// Assigns a default name to a single D3D12 object to aid in identification
// of the object during graphics debugging.
// @param x - pointer to a D3D12 object type.
#define SET_D3D_DEBUG_NAME(x) \
	x->SetName(L#x);
#else
#define SET_D3D_DEBUG_NAME(x)
#endif


#if defined(_DEBUG)
// Assigns default names to an array of D3D12 objects to aid in identification
// of the objects during graphics debugging.
// @param x - array of ComPtr<T> where T is a D3D12 object type.
// @param n - size of array.
#define SET_D3D_DEBUG_NAMES(x, n) \
	do { \
		for (unsigned i(0); i < (n); ++i) { \
			x[i]->SetName(toWideString(x) L"[" toWideString(i) L"]"); \
		} \
	} while(0)
#else
#define SET_D3D_DEBUG_NAMES(x, n)
#endif


#if defined(_DEBUG)
// Sets a specific name for a single D3D12 object to aid in identification
// of the object during graphics debugging.
// @param x - ComPtr<T> where T is a D3D12 object type.
#define D3D12_SET_NAME(x, name) \
	x->SetName(name);
#else
#define D3D12_SET_NAME(x, name)
#endif

// Force runtime to break with optional message.
#if defined(_DEBUG)
#define ForceBreak(message) \
	do { \
		if (message) { \
			LOG_ERROR(message); \
		} \
		__debugbreak(); \
	} while(0)
#else
#define ForceBreak(message)
#endif


