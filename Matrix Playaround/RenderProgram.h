#pragma once
#include <list>
#include <glm/glm.hpp>

#include "Global.h"

#include "ShaderProgram.h"
#include "ModelBase.h"

class RenderProgram : public ShaderProgram
{
public:
    RenderProgram();
    void updateValue();
    void render(ModelBase* data);
    void render2(ModelBase* data);
    void render3(ModelBase* data);
    void render4(ModelBase* data);
    void renderBox(ModelBase* data);
    Camera* camTest;
};

