//
// GameObject.hpp
//
#pragma once

#include "Core/Common.hpp"

#include "Graphics/RenderComponent.hpp"

struct GameObject {
	float position[3];
	//quat pose;

	RenderComponent render;
};



