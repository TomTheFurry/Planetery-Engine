#pragma once

#include <glm/glm.hpp>

#include "Anchors.h"

class GridBaseRenderer
{
public:
	GridBaseRenderer(Anchor* a);
	virtual ~GridBaseRenderer();
	uint gdId;
	Anchor* anchor;
	virtual bool needZoomIn();
	virtual bool needZoomOut();
	virtual void prerender();
	virtual void render();

};