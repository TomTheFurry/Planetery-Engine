
#include <iostream>
#include <list>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderGui.h"

RenderGui::RenderGui() :
    ShaderProgram("shader/gui.vert", "shader/gui.frag")
{

}

void RenderGui::updateValue()
{
    glEnable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    glDisable(GL_BLEND);
    //glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_LINE_SMOOTH);
    glLineWidth(2.0f);
}


void RenderGui::render(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}

void RenderGui::postRender()
{
    glLineWidth(1.0f);
}
