#include "pch.h"

#include "Graphics/ShaderUtils.hpp"

CompiledShader::CompiledShader ()
	: byteCode{nullptr, 0}
{

}

//---------------------------------------------------------------------------------------
CompiledShader::~CompiledShader ()
{
	delete byteCode.pShaderBytecode;
}


//---------------------------------------------------------------------------------------
#include <fstream>

void LoadCompiledShaderFromFile (
	const char * csoFile,
	std::shared_ptr<CompiledShader> & shaderSource
) {
	// Open file, and advance read position to end of file.
	std::ifstream file (csoFile, std::ios::in | std::ios::binary | std::ios::ate);
	if (!file.is_open ()) {
		ForceBreak ("Unable to open shader .cso file");
	}

	// Get current read position within input stream, which is the size in bytes of file.
	std::streamsize size = file.tellg ();

	// Reposition read pointer to beginning of file.
	file.seekg (0, std::ios::beg);

	char * bytes = new char[size];

	// Read file into bytes array.
	if (!(file.read (bytes, size))) {
		ForceBreak ("Unable to read all bytes within shader .cso file.");
	}

	shaderSource = std::make_shared<CompiledShader> ();

	shaderSource->byteCode.BytecodeLength = size;
	shaderSource->byteCode.pShaderBytecode = bytes;
}
