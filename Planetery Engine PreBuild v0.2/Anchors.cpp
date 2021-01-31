
#include <glm/glm.hpp>

using vec3 = glm::vec3;
using mat4 = glm::mat4;
using uint = unsigned int;
using ulint = unsigned long long;

#include "Anchors.h"


//Anchor

Anchor::Anchor(uint aId, uint gT, uint gId, vec3 p, vec3 s) {
	anchorId = aId;
	gridType = gT;
	gridId = gId;
	pos = p;
	speed = s;
}



//Player

Player::Player(uint aId, uint gT, uint gId, vec3 p, vec3 s) : Anchor(aId, gT, gId, p, s) {
}


