#pragma once
#include <list>
#include <glm/glm.hpp>

#include "Global.h"

#include "ShaderProgram.h"
#include "ModelBase.h"

class RenderProgramA : public ShaderProgram
{
public:
    RenderProgramA();
    void updateValue();
    void render(ModelBase* data);
    void postRender();
    Camera* camTest;
    uint fbo;
    uint colorOut;
    ShaderProgram* postProcess;
    ShaderProgram* postProcess2;
    ivec2 fboSize;
    float brightness;

};

