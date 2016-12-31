//
// GameObject.hpp
//
#pragma once

#include "Graphics/RenderComponent.hpp"

struct GameObject {
	float position[3];
	//quat pose;

	RenderComponent render;
};



