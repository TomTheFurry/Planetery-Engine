#pragma once
#include <iostream>
#include <list>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include "RenderProgram.h"
#include "ShaderProgram.h"

RenderProgram::RenderProgram() : ShaderProgram("shader/lighting.vert", "shader/lighting.frag")
{
    time = 0.0f;
    setup();
    light = Light();
}

void RenderProgram::updateValue()
{
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    time = (float)glfwGetTime();
}


void RenderProgram::render(RenderData data, RenderData camera)
{
    enable();
    setVec3("lightCoord", light.pos);
    setVec3("lightColorA", light.ambient);
    setVec3("lightColorD", light.diffuse);
    setVec3("lightColorS", light.specular);
    setFloat("lightPower", light.power);

    glBindVertexArray(data.vaoId);
    setMat4("coordMatrix", *camera.matrixs[0]);
    setMat4("projectMatrix", *camera.matrixs[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.veoId);

    setMat4("objectMatrix", *data.matrixs[0]);
    setMat4("normalMatrix", *data.matrixs[1]);
    glDrawElements(GL_TRIANGLES, data.veoLength, GL_UNSIGNED_INT, 0);
}

void RenderProgram::render(std::list<RenderData> data, RenderData camera)
{
    enable();
    setVec3("lightCoord", light.pos);
    setVec3("lightColorA", light.ambient);
    setVec3("lightColorD", light.diffuse);
    setVec3("lightColorS", light.specular);
    setFloat("lightPower", light.power);

    std::list<RenderData>::iterator i = data.begin();
    setMat4("coordMatrix", *camera.matrixs[0]);
    setMat4("projectMatrix", *camera.matrixs[1]);
    glBindVertexArray(i->vaoId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, i->veoId);

    for (i; i != data.end(); i++) {
        setMat4("objectMatrix", *i->matrixs[0]);
        setMat4("normalMatrix", *i->matrixs[1]);
        glDrawElements(GL_TRIANGLES, i->veoLength, GL_UNSIGNED_INT, 0);
    }
}

void RenderProgram::setup()
{
    logger << "Started to load texture...\n" << "\n";
    //load texture
    int width, height, nrChannels;
    unsigned char* data = stbi_load("bin/sample.png", &width, &height, &nrChannels, 0);
    if (!data)
    {
        logger << "Failed to load texture:\n" << stbi_failure_reason() << "\n";
        throw;
    }

    glGenTextures(1, &texture0);
    glBindTexture(GL_TEXTURE_2D, texture0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    logger << "Loaded texture:\n" << std::to_string(texture0) << "\n";

    data = NULL;
    data = stbi_load("bin/sample2.png", &width, &height, &nrChannels, 0);
    if (!data)
    {
        logger << "Failed to load texture:\n" << stbi_failure_reason() << "\n";
        throw;
    }

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);
    logger << "Loaded texture:\n" << std::to_string(texture1) << "\n";


    setInt("texture0", 0);
    setInt("texture1", 1);
}