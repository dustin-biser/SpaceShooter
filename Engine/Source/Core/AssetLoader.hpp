//
// AssetLoader.hpp
//
#pragma once

#include "Graphics/RenderComponent.hpp"

struct ObjAsset {
	MeshComponent mesh;
	Texture texture;
};

//TODO (Dustin) - Change all AssetIds to uint32 and hash Asset string names using CRC-32 algorithm.
// Want to so something like AssetLoader::load(AssetId("shipModel"), ship);
typedef const char * AssetId;

class AssetLoader {
public:
	template <typename AssetReturned>
	static void load (AssetId assetId, AssetReturned * out);

};


#include "Core/AssetLoader.inl"
