#pragma once
#include <list>
#include <glm/glm.hpp>

#include "ShaderProgram.h"
#include "ModelBase.h"

class RenderProgram : public ShaderProgram
{
public:
    RenderProgram();
    void objectMode();
    void outlineMode();
    void render(ModelBase* data);
};

