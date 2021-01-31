
#include <iostream>
#include <list>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <stb/stb_image.h>
#include <glm/gtc/matrix_transform.hpp>

#include "RenderProgramA.h"

RenderProgramA::RenderProgramA() :
    ShaderProgram("shader/starPreProcess.vert", "shader/starPreProcess.frag")
{
    fboSize = ivec2(1920, 1080);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    //color buffer
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &colorOut);
    glBindTexture(GL_TEXTURE_2D, colorOut);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, fboSize.x, fboSize.y,
        0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorOut, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    /* Not using this one as depth data is not needed in post processing
    //depth buffer
    glGenTextures(1, &depthOut);
    glBindTexture(GL_TEXTURE_2D, depthOut);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, global->windowSize.x, global->windowSize.y,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthOut, 0);
    */

    //depth renderbuffer object
    /*
    uint depthRbo;
    glGenRenderbuffers(1, &depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
        global->windowSize.x, global->windowSize.y);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRbo);
    */

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        logger << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
    else
        logger << "Framebuffer creation completed.\n";

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    postProcess = new ShaderProgram("shader/starPostProcessCross.vert", "shader/starPostProcessCross.frag");

    postProcess->enable();
    
    postProcess->setIvec2("windowSize", global->windowSize);

    brightness = 0.0f;
}

void RenderProgramA::updateValue()
{
    glViewport(0, 0, fboSize.x, fboSize.y);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    enable();
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDisable(GL_STENCIL_TEST);
}


void RenderProgramA::render(ModelBase* data)
{
    enable();
    glBindVertexArray(data->gVao());
    mat4 mv = global->activeCamera->getMatViewport();
    mat4 mp = global->activeCamera->getMatProject();
    setMat4("viewportMatrix", mv);
    setMat4("projectMatrix", mp);
    uint veoId = data->gVeo();
    if (veoId == uint(-1)) {
        glDrawArrays(data->gType(), 0, data->gLength());
    }
    else {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data->gVeo());
        glDrawElements(data->gType(), data->gLength(), GL_UNSIGNED_INT, 0);
    }
}

void RenderProgramA::postRender()
{
    const uint SAMPLE = 100000;

    float total = 0.0f;

    float* data = new float[fboSize.x*fboSize.y];

    glReadPixels(0, 0, fboSize.x, fboSize.y, GL_LUMINANCE, GL_FLOAT, data);

    SeededRng rng(std::rand());
    for (uint i = 0; i < SAMPLE; i++) {
        total += pow(data[uint(rng.next(0, fboSize.x * fboSize.y))],1);
    }
    total /= SAMPLE;
    logger << "Total luminance: " << total << "\n";
    brightness = glm::max(total-0.2f, 0.0f);


    glViewport(0, 0, global->windowSize.x, global->windowSize.y);
    postProcess->enable();
    glActiveTexture(GL_TEXTURE0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_MULTISAMPLE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glDisable(GL_DEPTH_TEST);
    //glDepthFunc(GL_LEQUAL);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    postProcess->setFloat("brightness", brightness);


    glBindTexture(GL_TEXTURE_2D, colorOut);

    glBindVertexArray(boxVaoId);
    glDrawArrays(ShaderProgram::vaoMode, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);

}
