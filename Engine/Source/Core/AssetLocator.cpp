//
// AssetLocator.cpp
//
#include "pch.h"

#include "Core/AssetLocator.hpp"

//---------------------------------------------------------------------------------------
#include <libloaderapi.h>
#include <cstring>

namespace
{
	void GetWorkingDir (
		char * path,
		uint pathSize
	) {
		ASSERT (path);

		ulong size = GetModuleFileName (nullptr, path, pathSize);
		if (size == 0 || size == pathSize) {
			// Method failed or path was truncated.
			__debugbreak ();
		}

		// Get pointer to last backslash in path.
		char * lastSlash = std::strrchr (path, '\\');
		if (lastSlash) {
			// Insert null char after last backslash.
			*(lastSlash + 1) = '\0';
		}
	}
}

//---------------------------------------------------------------------------------------
#include <string>

std::string GetAssetPath (
	const char * assetName
) {
	ASSERT (assetName);

	//TODO (Dustin) - For now assume assets are in working dir. Later we want asset path lookup table.

	std::string assetPath;
	{
		char pathBuffer[512];
		GetWorkingDir (pathBuffer, _countof (pathBuffer));
		assetPath = std::string (pathBuffer) + std::string (assetName);
	}

	// Convert to wstring.
	std::string result;
	result.assign (assetPath.begin (), assetPath.end ());

	return result;
}
