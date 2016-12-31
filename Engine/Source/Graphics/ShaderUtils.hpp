//
// ShaderUtils.hpp
//
#pragma once

#include <d3d12.h>
#include <memory>

class CompiledShader {
public:
	CompiledShader ();

	~CompiledShader ();

	D3D12_SHADER_BYTECODE byteCode;
};


/// Loads compiled shader object (cso) bytecode file into shaderSource.
void LoadCompiledShaderFromFile (
	const char * csoFile,
	std::shared_ptr<CompiledShader> & shaderSource
);
