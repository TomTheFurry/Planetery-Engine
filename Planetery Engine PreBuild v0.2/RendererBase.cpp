#include <glm/glm.hpp>

#include "Anchors.h"

#include "RendererBase.h"

using vec3 = glm::vec3;
using mat4 = glm::mat4;
using uint = unsigned int;
using ulint = unsigned long long;



GridBaseRenderer::GridBaseRenderer(Anchor* a) {
	gdId = a->gridId;
	anchor = a;
}

GridBaseRenderer::~GridBaseRenderer()
{
}

bool GridBaseRenderer::needZoomIn() {
	return false;
}

bool GridBaseRenderer::needZoomOut() {
	return false;
}

void GridBaseRenderer::prerender() {
}

void GridBaseRenderer::render() {
}