//
// AssetLoader.inl
//
#include <memory>

#include "Core/AssetLoader.hpp"
#include "Core/AssetLocator.hpp"

//---------------------------------------------------------------------------------------
template <typename AssetReturned>
void AssetLoader::load (
	AssetId assetId,
	AssetReturned * out
) {

}

//---------------------------------------------------------------------------------------
#include "Graphics/ShaderUtils.hpp"
#include <unordered_map>

template <>
inline void AssetLoader::load (
	AssetId assetId,
	std::shared_ptr<CompiledShader> * outShader
) {
	assert(outShader);

	// Store of previously loaded shaders
	static std::unordered_map<AssetId, std::shared_ptr<CompiledShader>> loadedShaders;

	if (loadedShaders.count (assetId) == 0) {
		std::string byteCodePath = GetAssetPath (assetId);
		LoadCompiledShaderFromFile (byteCodePath.c_str(), *outShader);

		// Store a copy for future retrival
		loadedShaders[assetId] = *outShader;
	}
	else {
		*outShader = loadedShaders.at (assetId);
	}
}
