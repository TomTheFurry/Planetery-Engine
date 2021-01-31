#include <iostream>
#include <random>
#include <windows.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Global.h"
#include "GridGalaxies.h"



// Absolute Position
// X: left->right  Y: back->front  Z: down->up

// Relative Position
// X: left->right  Y: back->front  Z: down->up

// Rotation
// X: yaw (XY)  Y: pitch (YZ)  Z: roll (XZ)



void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}



void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 0.001f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed = 0.002f;

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        auto g = global->worldManager->getGridNoLoad(global->main->gridType, global->main->gridId);
        if (g->type == 0) {
            auto gg = (GridGalaxies*)g;
            gg->TranslationTypeA::sVol(vec3(0.0f));
        }
        else {
            auto gg = (GridGalaxy*)g;
            gg->TranslationTypeA::sVol(vec3(0.0f));
        }
    } else {
        vec3 acceleration = vec3(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            acceleration += vec3(0.0f, 1.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            acceleration += vec3(0.0f, -1.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            acceleration += vec3(-1.0f, 0.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            acceleration += vec3(1.0f, 0.0f, 0.0f);
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
            acceleration += vec3(0.0f, 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
            acceleration += vec3(0.0f, 0.0f, -1.0f);

        vec3 rotation = vec3(0.0f);
        if (acceleration != vec3(0.0f))
            acceleration = glm::normalize(acceleration);

        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
            rotation += vec3(0.0f, 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
            rotation += vec3(0.0f, 0.0f, -1.0f);

        auto g = global->worldManager->getGridNoLoad(global->main->gridType, global->main->gridId);
        if (g->type == 0) {
            auto gg = (GridGalaxies*)g;
            gg->aAcc(acceleration * cameraSpeed);
            gg->TranslationTypeA::aRot(rotation);
        }
        else {
            auto gg = (GridGalaxy*)g;
            gg->aAcc(acceleration * cameraSpeed * 200.0f);
            gg->TranslationTypeA::aRot(rotation);
            auto ggs = (GridGalaxies*)global->worldManager->getGridNoLoad(0, gg->ugdId);
            ggs->TranslationTypeA::aRot(rotation);
        }

    }
}



glm::vec2 mouseLastPos = glm::vec2(1920, 1080);
bool mousePosGot = false;
const float sensitivity = 0.2f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!mousePosGot) {
        mouseLastPos = glm::vec2(xpos, ypos);
        mousePosGot = true;
    }
    else {
        glm::vec2 mouseOffset = glm::vec2(xpos, ypos) - mouseLastPos; //note that y offset hast to be reversed
        mouseOffset.y = -mouseOffset.y;
        mouseLastPos = glm::vec2(xpos, ypos);
        mouseOffset *= sensitivity;
        auto g = global->worldManager->getGridNoLoad(global->main->gridType, global->main->gridId);
        if (g->type == 0) {
            auto ggs = (GridGalaxies*)g;
            ggs->TranslationTypeA::aRot(vec3(mouseOffset.x, mouseOffset.y, 0.0f));
        }
        else {
            auto gg = (GridGalaxy*)g;
            gg->TranslationTypeA::aRot(vec3(mouseOffset.x, mouseOffset.y, 0.0f));
            auto ggs = (GridGalaxies*)global->worldManager->getGridNoLoad(0,gg->ugdId);
            ggs->TranslationTypeA::aRot(vec3(mouseOffset.x, mouseOffset.y, 0.0f));
        }
    }
}

void error_callback(int error, const char* description)
{
    logger << "Error:" << error << "\n" << std::string(description);
    if (error == 65543) {
        MessageBox(NULL, LPCWSTR(u"Graphic Driver Not Supported. Needs at least OpenGL Version 4.6."),
            LPCWSTR(u"Error"), 0x00000030L);
    }
    else {
        MessageBox(NULL, LPCWSTR(u"Unknown Error from glfw. Possibly a driver issue. Read log.txt for futher details."),
            LPCWSTR(u"Error"), 0x00000030L);
    }
}

int main() {
    logger << "Stared Program... \n";
    GLFWwindow* window;
    try {
        stbi_set_flip_vertically_on_load(true);
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_SAMPLES, 32);
        glfwSetErrorCallback(error_callback);
        window = glfwCreateWindow(global->windowSize.x, global->windowSize.y, "Planetery Demo", NULL, NULL);
        if (window == NULL)
        {
            logger << "Failed to create GLFW window" << "\n";


            glfwTerminate();
            return -1;
        }

        glfwMakeContextCurrent(window);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            logger << "Failed to initialize GLAD" << "\n";
            return -1;
        }
        glViewport(0, 0, global->windowSize.x, global->windowSize.y);

        //this links to glfwMakeContextCurrent()
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        //this locks the cursor into the window and hide it
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);
    }
    catch (std::exception e) {
        logger << "Uncaught Error: " << e.what();
        return -1;
    }
    catch (...) {
        logger << "Uncaght Throwable!";
        return -1;
    }

    //Model Data
    //Removed


    //Gen Manager
    global->mapFileManager = new MapFileManager();
    global->worldManager = new WorldManager();
    global->renderManager = new RenderManager();


    //Gen World
    global->worldManager->loadMap(0);
    global->worldManager->startWorld();
    global->worldManager->tickAnchor();
    //Gen Model and Renderer
    logger << "Started to load renderer and run one frame..." << "\n";
    global->renderManager->setRenderTarget(&global->worldManager->anchors[0]);
    logger << "Running frame..." << "\n";
    global->renderManager->renderTarget();
    logger << "Conpleted first frame." << "\n";


    logger << "Finished init. Starting loop..." << "\n";
    //Loop
    while (!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);

        //tick
        global->worldManager->tickAnchor();

        //render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        global->renderManager->renderTarget();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}