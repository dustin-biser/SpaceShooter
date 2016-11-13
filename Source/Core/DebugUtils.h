//
// DebugUtils.h
//
#pragma once

#include <cwchar>
#include <debugapi.h>

#define LOG_BUFFER_LENGTH 256
#define LOG_LEVEL_INFO L"Log Info: "
#define LOG_LEVEL_WARNING L"Log Warning: "
#define LOG_LEVEL_ERROR L"Log Error: "

#if defined(_DEBUG)
#define LOG(levelWString, format, ...) \
	do { \
		wchar_t buffer[LOG_BUFFER_LENGTH]; \
		int wcharsWritten = swprintf(buffer, LOG_BUFFER_LENGTH, levelWString); \
		wcharsWritten += swprintf(buffer + wcharsWritten, LOG_BUFFER_LENGTH, L##format, __VA_ARGS__); \
		swprintf(buffer + wcharsWritten, LOG_BUFFER_LENGTH, L"\n"); \
		OutputDebugStringW(buffer); \
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

