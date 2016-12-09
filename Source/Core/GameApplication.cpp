//
// GameApplication.cpp
//

#include "pch.h"

#include "Core/GameApplication.hpp"
#include "Core/GameObject.hpp"
#include "Core/AssetLoader.hpp"

#include "Graphics/D3D12Renderer.hpp"


//---------------------------------------------------------------------------------------
GameApplication::GameApplication (
	uint windowWidth,
	uint windowHeight,
	const char * windowTitle
) 
	: _windowWidth(windowWidth),
	  _windowHeight(windowHeight),
	  _windowTitle(windowTitle)
{

}

//---------------------------------------------------------------------------------------
const char * GameApplication::getWindowTitle () const
{
	return _windowTitle;
}

//---------------------------------------------------------------------------------------
uint GameApplication::getWindowWidth () const
{
	return _windowWidth;
}

//---------------------------------------------------------------------------------------
uint GameApplication::getWindowHeight () const
{
	return _windowHeight;
}

//---------------------------------------------------------------------------------------
void GameApplication::initialze (
	HWND hWindow
) {
	// Allocate instance for D3D12Renderer.
	_renderer = std::make_shared<D3D12Renderer> ();
	_renderer->initialize (hWindow);

	GameObject ship;

	ObjAsset objAsset;
	AssetLoader::load ("low_poly_ship", &objAsset);

	ShaderGroup shader;
	//AssetLoader::load ("defaultVS", &shader.vertexShader);
	//AssetLoader::load ("defaultPS", &shader.pixelShader);

	//Material material;
	//material.shader = shader;
	//material.texture = objAsset.texture;

	//ship.addComponent (objAsset.mesh);
	//ship.addComponent (material);

	//_renderer->addGameObject (ship);
}

//---------------------------------------------------------------------------------------
void GameApplication::keyDown (
	uint8 virtualKey
) {
	_inputHandler.keyDown(virtualKey);
}

//---------------------------------------------------------------------------------------
void GameApplication::keyUp (
	uint8 virtualKey
) {
	_inputHandler.keyUp(virtualKey);
}

//---------------------------------------------------------------------------------------
void GameApplication::update ()
{
	_renderer->render ();
	_renderer->present ();
}

