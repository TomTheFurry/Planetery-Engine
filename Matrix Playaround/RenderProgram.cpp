
#include <iostream>
#include <list>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderProgram.h"

RenderProgram::RenderProgram() : ShaderProgram("shader/basic.vert", "shader/basic.frag")
{
    camTest = new Camera(60, vec3(0.0, -10.0, 0.0), vec3(0));
    camTest->min = 0.0f;
    camTest->max = 1.0f;
}

void RenderProgram::updateValue()
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
    setMat4("viewportMatrix", mat4(1.0));
    setMat4("projectMatrix", mat4(1.0));
    setMat4("testMatrix", mat4(1.0));
    setMat4("colorMatrix", mat4(0.5));
    camTest->update();
}


void RenderProgram::render(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    setMat4("viewportMatrix", mv);
    setMat4("projectMatrix", mp);
    setMat4("colorMatrix", mat4(0.1));
    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}

void RenderProgram::render2(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    setMat4("viewportMatrix", mv);
    setMat4("projectMatrix", mat4(vec4(00.1, 0, 0, 0), vec4(0, 00.1, 0, 0), vec4(0, 0, 00.1, 0), vec4(0, 0, 0, 1)));

    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}

void RenderProgram::render3(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    mat4 smv = camTest->getMatViewport();
    smv[3][3] = 1;
    mat4 smp = camTest->getMatProject();
    smp[3][3] = 1;
    mat4 test = smp;
    float t = glfwGetTime();
    mat4 sm = smv * ((test * mat4(sin(t) / 2.0 + 0.5)) + mat4(0.5 - sin(t) / 2.0));

    sm[3] = vec4(0.0);
    sm[0][3] = 0;
    sm[1][3] = 0;
    //mps[2][3] = 0;
    sm[3][3] = 1;


    setMat4("viewportMatrix", mp * mv * sm);
    setMat4("projectMatrix", mat4(1.0f));
    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}

void RenderProgram::render4(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    mat4 smv = camTest->getMatViewport();
    mat4 smp = camTest->getMatProject();
    //mp = mat4(mat3(mp));
    setMat4("viewportMatrix", mv);
    setMat4("projectMatrix", mp);
    float t = glfwGetTime();


    mat4 flip = mat4(vec4(1, 0, 0, 0), vec4(0, 1, 0, 0), vec4(0, 0, -1, 0), vec4(0, 0, 0, 1));
    mat4 test = flip * glm::perspective(glm::radians(90.0f), 1.0f, 5.0f, 15.0f) * smv;

    test = (test * mat4(sin(t) / 2.0 + 0.5) + smv * (mat4(0.5 - sin(t) / 2.0)));



    setMat4("testMatrix", test);
    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}

void RenderProgram::renderBox(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    setMat4("viewportMatrix", mv);
    setMat4("projectMatrix", mp);
    setMat4("colorMatrix", mat4(0.3));

    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}