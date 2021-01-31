#include <iostream>
#include <bitset>
#include <chrono>
#include <Vector>
#include <thread>
#include <mutex>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>



#include "Global.h"

#include "Physic.h"

#include "RenderProgram.h"
//#include "RenderGui.h"
#include "ModelBase.h"
#include "ShaderProgram.h"
#include "RenderThread.h"
#include "Bridge.h"



// Absolute Position
// X: left->right  Y: back->front  Z: down->up

// Relative Position
// X: left->right  Y: back->front  Z: down->up

// Rotation
// X: yaw (XY)(left to right)  Y: pitch (YZ)(down to up)  Z: roll (XZ)(clockrise)


float camYaw = -90.0f;
float camPitch = 0.0f;

RenderProgram* r;
//RenderGui* rGui;
ModelCube* cube;
ModelCube2* cubePointer;
ModelCubeStatic* floorCube;
ModelCubeStatic* floorCube2;
ModelGrid* grid;
ModelGrid* grid2;
ModelGui* gui;
ModelPoints* pStar;
ModelFloor* basicFloor;
CameraTargetMovable* cam1;
CameraFreeStyle* cam2;
bool isCam1 = true;
Scene* scene;
GLFWwindow* window = nullptr;

std::vector<ModelCube*> cubes;
std::vector<ModelConvex*> cubesConvex;
std::vector<ModelSphere*> balls;

std::thread physicThread;
std::thread renderThread;

std::mutex bufferLock;
std::vector<ModelBase*> initQueue;
std::vector<ModelBase*> destQueue;



void framebuffer_size_callback(GLFWwindow* _, int width, int height)
{
    glViewport(0, 0, width, height);
}


bool mousePressed = false;
bool mousePressed2 = false;
bool tabPressed = false;

uint spawnMode = 1;

void processInput(GLFWwindow* _)
{
    if (!global) return;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 0.001f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed = 0.002f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->move(vec3(0.0f, cameraSpeed, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->move(vec3(0.0f, -cameraSpeed, 0.0f));

    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->move(vec3(-cameraSpeed, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->move(vec3(cameraSpeed, 0.0f, 0.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->move(vec3(0.0f, 0.0f, cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->move(vec3(0.0f, 0.0f, -cameraSpeed));
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->rotate(vec3(0.0f, 0.0f, 1.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        CameraMovable* camera = global->activeCamera;
        camera->rotate(vec3(0.0f, 0.0f, -1.0f));
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        CameraFloating* camera = dynamic_cast<CameraFloating*>(global->activeCamera);
        if (camera != nullptr)
            camera->slowdown(0.5f);
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressed) {
        if (isCam1) {
            global->activeCamera = cam2;
            isCam1 = false;
        }
        else {
            cam1->t->sPos(cam2->t->gPos());
            global->activeCamera = cam1;
            isCam1 = true;
        }
        tabPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_RELEASE && tabPressed) {
        tabPressed = false;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        spawnMode = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        spawnMode = 2;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        spawnMode = 3;
    }

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS && !mousePressed) {
        switch (spawnMode)
        {
        case 1:
            cubesConvex.push_back(addModel<ModelConvex>(global->activeCamera->gPos(), scene));
            break;
        case 2:
            cubes.push_back(addModel<ModelCube>(global->activeCamera->gPos(), scene));
            break;
        case 3:
            balls.push_back(addModel<ModelSphere>(global->activeCamera->gPos(), scene));
            break;
        default:
            break;
        }
        logger << "Made new object.\n";
        mousePressed = true;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE && mousePressed) {
        mousePressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS && !mousePressed2) {
        //logger << "Deleting cube.\n";
        auto it = cubes.begin();
        while (it != cubes.end()) {
            if ((*it)->isInside(global->activeCamera->gPos())) {
                delModel(*it);
                it = cubes.erase(it);
                logger << "Deleted a cube.\n";
            } else {
                it++;
            }
        }

        mousePressed2 = true;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE && mousePressed2) {
        mousePressed2 = false;
    }

    
}



glm::vec2 mouseLastPos = glm::vec2(1920, 1080);
bool mousePosGot = false;
const float sensitivity = 0.2f;

void mouse_callback(GLFWwindow* _, double xpos, double ypos) {
    if (!mousePosGot) {
        mouseLastPos = glm::vec2(xpos, ypos);
        mousePosGot = true;
    }
    else {
        glm::vec2 mouseOffset = glm::vec2(xpos, ypos) - mouseLastPos; //note that y offset hast to be reversed
        mouseOffset.y = -mouseOffset.y;
        mouseLastPos = glm::vec2(xpos, ypos);
        mouseOffset *= sensitivity;
        global->activeCamera->rotate(vec3(mouseOffset.x, mouseOffset.y, 0.0f));
    }
}

void glfwErrorCallback(int errorCode, const char* text) {
    logger << "GLFW ERROR::\n";
    logger << "Error Code: " << std::to_string(errorCode) << "\n";
    logger << std::string(text) << "\n";
    if (errorCode == 65543) {
        logger << "\n\nsThis software requires OpenGL Version 4.6 or higher!!!\n\n";
    }
}



float renderDelta = 0;
float renderStep = 1. / 60.;

/*
static void renderMain() {
    logger << "\n\n\nPlanetery Demo Started.\n";

    glfwSetErrorCallback(glfwErrorCallback);
    stbi_set_flip_vertically_on_load(true);
    glfwInit();
    RenderThread::setWindownHint();
    window = glfwCreateWindow(global->windowSize.x, global->windowSize.y, "Planetery Demo", NULL, NULL);
    if (window == NULL)
    {
        logger << "Failed to create GLFW window" << "\n";
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //Context Current Needed
    {
        logger << "Failed to initialize GLAD" << "\n";
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //Main thread only
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Main thread only
    glfwSetCursorPosCallback(window, mouse_callback); //Main thread only

    glfwMakeContextCurrent(NULL); //Hand over context to the new render thread
    RenderThread::create(window);
    
    

    //if (renderDelta >= renderStep) {
        //renderDelta = fmod(renderDelta,renderStep);
        //pre render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render

        r->objectMode();
        r->render(cube);
        //r->render(floorCube);
        //r->render(floorCube2);
        r->render(cubePointer);
        r->render(basicFloor);
        for (auto c : cubes) {
            r->render(c);
        }
        r->outlineMode();
        r->render(grid);

        rGui->updateValue();
        rGui->render(gui);
        rGui->postRender();


        //endTick
        glfwSwapBuffers(window);
        glfwPollEvents();
    //}
}*/


//BUG WARNING!!!  If the outer loop is too fast, the logicTickDelta check (if still a double)
// will round down the delta and cause low tick rate!!!! Use ulint instead!
// However, overflow may causes problems down in the line!!!
ulint logicTickDelta = 0;
ulint logicTickStep = (1. / 120. * 1000000000.);

static void logicTick(ulint delta) {
    logicTickDelta += delta;

    if (logicTickDelta >= logicTickStep) {
        
        //input
        processInput(window);

        //logic update
        if (isCam1) {
            cam1->update();
            cubePointer->pos = cam1->gPos();
        }
        else {
            cam2->update();
        }

        logicTickDelta -= logicTickStep;
    }
}


int main() {
    logger << "\n\n\nPlanetery Demo Started.\n";

    glfwSetErrorCallback(glfwErrorCallback);
    stbi_set_flip_vertically_on_load(true);
    glfwInit();
    RenderThread::setWindownHint();
    window = glfwCreateWindow(global->windowSize.x, global->windowSize.y, "Planetery Demo", NULL, NULL);
    if (window == NULL)
    {
        logger << "Failed to create GLFW window" << "\n";
        glfwTerminate();
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //Context Current Needed
    {
        logger << "Failed to initialize GLAD" << "\n";
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //Main thread only
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); //Main thread only
    glfwSetCursorPosCallback(window, mouse_callback); //Main thread only

    glfwMakeContextCurrent(NULL); //Hand over context to the new render thread
    logger << "MainThread: Basic initialization done. Starting RenderThread...\n";
    RenderThread::create(window);

    //PhysX Setup
    logger << "MainThread: Brench off done. Started to load physics...\n";
    PhysicAPI::init();
    // Absolute Position
    // X: left->right  Y: back->front  Z: down->up
    scene = new Scene(1.f, vec3(0.0f, 0.0f, -20.0f));

    logger << "MainThread: Started to make singleton models..." << "\n";
    //TODO: Better "addModel()" method
    //cube = addModel<ModelCube>(scene);
    cubePointer = addModel<ModelCube2>();
    //floorCube = new ModelCubeStatic(vec3(0.f, 0.f, -2.f), scene);
    //floorCube2 = new ModelCubeStatic(vec3(2.f, 0.f, 0.f), scene);
    grid = addModel<ModelGrid>(20, 1.0f, vec3(0.7f));
    grid2 = addModel<ModelGrid>(10, 1.0f, vec3(1.0f, 0.0f, 0.0f));
    pStar = addModel<ModelPoints>(100000, vec3(0), vec3(10), vec3(1.0));
    gui = addModel<ModelGui>();
    basicFloor = addModel<ModelFloor>(scene);
    //rGui = addModel<RenderGui>();

    logger << "MainThread: Finished making models." << "\n";
    logger << "MainThread: Started making instances..." << "\n";
    cam1 = new CameraTargetMovable();
    cam2 = new CameraFreeStyle();
    global->activeCamera = cam1;


    logger << "MainThread: Finished init." << "\n";
    RenderThread::start();

    auto timerS = std::chrono::high_resolution_clock::now();
    auto timerE = std::chrono::high_resolution_clock::now();
    ulint delta = 1;
    ulint testdelta = 0;
    uint tickCount = 0;
    //Loop
    while (!glfwWindowShouldClose(window))
    {
        timerS = std::chrono::high_resolution_clock::now();

        logicTick(delta);
        PhysicAPI::tickAll(delta);

        tickCount++;
        testdelta += delta;
        if (testdelta >= 1000000000) {
            logger << "Cube count: " << cubes.size() << " Delta time: " << testdelta <<
                " Tick count: " << tickCount << "\n";
            testdelta -= 1000000000;
            tickCount = 0;
        }
        glfwPollEvents();
        timerE = std::chrono::high_resolution_clock::now();
        delta = (timerE - timerS).count();
    }
    logger << "MainThread: Stopping game. Calling all threads to exit.\n";
    RenderThread::get()->stop();
    PhysicAPI::cleanup();
    RenderThread::get()->join();
    logger << "MainThread: All side threads exited. Terminating windows...\n";

    glfwTerminate();
    return 0;
}
