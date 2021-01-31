#pragma once

#include <glm/glm.hpp>

#include "utility.h"

class Anchor
{
public:
	uint anchorId;
	uint gridType; //gridId: 0 = galaxy clusters, 1 = galaxy, 2 = star system, 3 = planet
	uint gridId;
	vec3 pos;
	vec3 speed;

	Anchor(uint aId, uint gridType, uint gridId, vec3 pos, vec3 speed);
};

class Player : public Anchor
{
public:
	Player(uint aId, uint gridType, uint gridId, vec3 pos, vec3 speed);
};