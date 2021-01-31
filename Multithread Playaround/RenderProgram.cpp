
#include <iostream>
#include <list>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderProgram.h"
#include "Global.h"

RenderProgram::RenderProgram() : ShaderProgram("shader/flatNormal.vert", "shader/flatNormal.frag")
{
}

void RenderProgram::objectMode()
{
    enable();
    glEnable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    setMat4("colorMatrix", mat4(1.0));
}

void RenderProgram::outlineMode()
{
    enable();
    glEnable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    //glEnable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    setMat4("colorMatrix", mat4(0.1));
}


void RenderProgram::render(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    setMat4("viewportMatrix", mv);
    setMat4("projectMatrix", mp);
    setMat4("objectMatrix", data->gMat());
    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}