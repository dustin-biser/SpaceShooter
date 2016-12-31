//
// IRenderer.hpp
//
#pragma once

#include <windef.h>

///	Interface representing a rendering system.
class IRenderer {
public:
	virtual ~IRenderer () {}

	virtual
	void initialize (
		HWND hWindow
	) = 0;

	/// Submits rendering of scene to attached framebuffer.
	virtual
	void render () = 0;

	/// Presents contents of framebuffer to screen.
	virtual
	void present () = 0;
};
