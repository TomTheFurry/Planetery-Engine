#pragma once
#include <list>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Data.h"
class RenderProgram : public ShaderProgram
{
public:
    RenderProgram();
    glm::mat4 coordMatrix;
    glm::mat4 camMatrix;
    void updateValue();
    void render(RenderData data, RenderData camera);
    void render(std::list<RenderData> data, RenderData camera);
    Light light;

private:
    float time;
    unsigned int texture0, texture1;
    void setup();
};

