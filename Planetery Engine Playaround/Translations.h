#pragma once

#include <glm/glm.hpp>

/*
TranslastionClass:
	Provides different method of storing, caching, calulating translations.
	Used in:
		Grid location based calulations
		Rendering matrixs
		Rendering model offsets
		Model object base

*/

using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat3 = glm::mat3;
using mat4 = glm::mat4;


class TranslationBase
{
public:
	TranslationBase(); //Does nothing
	virtual vec3 gPos(); //Override for get Pos
	virtual vec3 sPos(); //Override for set Pos
	virtual vec3 gVol(); //Override for get Vol (Speed)
	virtual vec3 sVol(); //Override for set Vol (Speed)
};

