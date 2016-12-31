//
// RenderComponent.hpp
//
#pragma once

#include <memory>

#include "Core/Types.hpp"
#include "Graphics/ShaderUtils.hpp"


struct Texture {
	uint width;
	uint height;

	uint bytesPerPixel;
	void * imageData;
};

/// A grouping of compiled shader programs that make up a shader pipeline.
/// Each CompiledShader member is reference counted for automatic memory cleanup.
struct ShaderGroup {
	std::shared_ptr<CompiledShader> vertexShader;
	std::shared_ptr<CompiledShader> pixelShader;
};

struct Material {
	Texture texture;
	ShaderGroup shaderGroup;
};

struct Vertex {
	float position[3];
	float normal[3];
	float uv_diffuse[2];
};

// Support 2^16 -> 65536 unique indices per MeshComponent.
typedef ushort Index;


struct MeshComponent {
	uint32 numVertices;
	uint32 numIndices;
	Vertex * vertices;
	Index * indices;
};

struct RenderComponent {
	MeshComponent mesh;
	Material material;
};



