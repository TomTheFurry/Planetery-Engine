#include <iostream>
#include <random>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "RenderProgram.h"
#include "Camera.h"
#include "Global.h"



// Absolute Position
// X: left->right  Y: back->front  Z: down->up

// Relative Position
// X: left->right  Y: back->front  Z: down->up

// Rotation
// X: yaw (XY)  Y: pitch (YZ)  Z: roll (XZ)






float camYaw = -90.0f;
float camPitch = 0.0f;





void printMatrix(glm::mat4 mat) {
    float* ptr = glm::value_ptr(mat);
    for (int i = 0; i < 16; i++) {

        logger << "[" << std::to_string(*(ptr + i)) << ", ";
        i++;
        logger << std::to_string(*(ptr + i)) << ", ";
        i++;
        logger << std::to_string(*(ptr + i)) << ", ";
        i++;
        logger << std::to_string(*(ptr + i)) << "]\n";

    }
    logger << "\n";
}

void printVec3(glm::vec3 vec) {
    logger << vec.x << ", " << vec.y << ", " << vec.z << "\n";
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInputOFF(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 0.1f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed = 0.2f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera->move(vec3(0.0f, cameraSpeed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera->move(vec3(0.0f, -cameraSpeed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera->move(vec3(-cameraSpeed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera->move(vec3(cameraSpeed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera->move(vec3(0.0f, 0.0f, cameraSpeed));
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera->move(vec3(0.0f, 0.0f, -cameraSpeed));
}




glm::vec2 mouseLastPos = glm::vec2(1920, 1080);
bool mousePosGot = false;
const float sensitivity = 0.2f;

void mouse_callbackOFF(GLFWwindow* window, double xpos, double ypos) {
    if (!mousePosGot) {
        mouseLastPos = glm::vec2(xpos, ypos);
        mousePosGot = true;
    }
    else {
        glm::vec2 mouseOffset = glm::vec2(xpos, ypos) - mouseLastPos; //note that y offset hast to be reversed
        mouseOffset.y = -mouseOffset.y;
        mouseLastPos = glm::vec2(xpos, ypos);
        mouseOffset *= sensitivity;
        camera->addRotation(vec3(mouseOffset.x, mouseOffset.y, 0.0f));
    }
}





int main() {
    stbi_set_flip_vertically_on_load(true);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(global->windowSize.x, global->windowSize.y, "Planetery Demo", NULL, NULL);
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


    //Model Data
    float cubeVertices[] = {
        //3, 2
        //0, 1
        //     XYZ                BaseColor       TexCoord          Normal
        //Front (-y)
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f, -1.0f,  0.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f, -1.0f,  0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f, -1.0f,  0.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f, -1.0f,  0.0f,
        //Back (+y)
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f,  1.0f,  0.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f,  1.0f,  0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f,  1.0f,  0.0f,
        //Left (-x)
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,  -1.0f,  0.0f,  0.0f,
        //Right (+x)
         1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   1.0f,  0.0f,  0.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   1.0f,  0.0f,  0.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   1.0f,  0.0f,  0.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   1.0f,  0.0f,  0.0f,
        //bottom (-z)
        -1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f,  0.0f, -1.0f,
         1.0f,  1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f,  0.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f,  0.0f, -1.0f,
        //top (+z)
        -1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f,   0.0f,  0.0f,  1.0f,
         1.0f, -1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 0.0f,   0.0f,  0.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  1.0f, 1.0f,   0.0f,  0.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,  1.0f, 1.0f, 1.0f,  0.0f, 1.0f,   0.0f,  0.0f,  1.0f
    };

    unsigned int cubeIndices[] = {
         0, 2, 3,   0, 1, 2,  //front
         4, 6, 7,   4, 5, 6,  //back
         8,10,11,   8, 9,10,  //left
        12,14,15,  12,13,14,  //right
        16,18,19,  16,17,18,  //bottom
        20,22,23,  20,21,22   //top
    };

    int cubeVertLength = sizeof(cubeVertices) / sizeof(float);
    int cubeIndLength = sizeof(cubeIndices) / sizeof(unsigned int);



    //Gen Model and Renderer
    logger << "Started to load renderer..." << "\n";


    logger << "Started to make models..." << "\n";

    galaxyLayer = new GalaxyLayer();

    //stars = new Stars(5);

    /*Light whiteLight = Light();
    whiteLight.pos = vec3(0.0f);
    whiteLight.ambient = vec3(0.0f,0.0f,1.0f);
    whiteLight.diffuse = vec3(0.0f,1.0f,0.0f);
    whiteLight.specular = vec3(1.0f,0.0f,0.0f);
    whiteLight.power = 10.0f;
    renderer->light = whiteLight;
    */

    //Model* lightCube = new Model((float*)cubeVertices, (int*)cubeIndices, cubeVertLength, cubeIndLength);
    //Model* cube = new Model((float*)cubeVertices,(int*)cubeIndices, cubeVertLength, cubeIndLength);
    //InstancedModel* cubes = new InstancedModel((float*)cubeVertices, (int*)cubeIndices, cubeVertLength, cubeIndLength);
    //InstancedModel* grids = new InstancedModel((float*)cubeVertices, (int*)cubeIndices, cubeVertLength, cubeIndLength);
    


    logger << "Finished making models." << "\n";
    logger << "Started making instances..." << "\n";

    /*
    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),    
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };
    int cubeInstances [10];
    for (int i = 0; i < 10; i++) {
        cubeInstances[i] = cubes->create(3.0f * cubePositions[i]);
        cubes->scale(cubeInstances[i], glm::vec3(1.0f, 0.2f, 0.5f));
    }

    int i = 0;
    for (int x = 0; x < 10; x++) {
        for (int y = 0; y < 10; y++) {
            for (int z = 0; z < 10; z++) {
                gridInstances[i] = grids->create(vec3(x,y,z));
                grids->scale(gridInstances[i], glm::vec3(0.2f, 0.2f, 0.2f));
                grids->addOffset(gridInstances[i], glm::vec3(-5.0f, -5.0f, -5.0f));
                i++;
            }
        }
        logger << "Making layer: " << x << "\n";
    }
    cube->setOffset(glm::vec3(0.0f, 5.0f, 0.0f));
    cube->setScale(glm::vec3(10.0f, 0.2f, 10.0f));
    cubes->setOffset(glm::vec3(0.0f, 10.0f, 0.0f));
    lightCube->scale(vec3(0.6f, 0.6f, 0.6f));
    */



    /*
    const int numOfStar = 1000;
    const int clusterSizeMean = 3;
    const float clusterOffsetMean = 5.0f;

    //global->galaxySeed = std::chrono::system_clock::now().time_since_epoch().count();
    global->galaxySeed = 100;

    std::default_random_engine generator(global->galaxySeed);
    std::normal_distribution<float> ran(0.0, 1.0);

    srand(global->galaxySeed);
    for (int i = 0; i < numOfStar; i++) {

        float posX = ran(generator) * 500.0f;
        float posY = ran(generator) * 500.0f;
        float posZ = ran(generator) * 500.0f;
        double r, g, b;

        int clusterSize = ceil(abs(ran(generator) + 1.0f) * clusterSizeMean);

        for (int i = 0; i < clusterSize; i++) {
            Star s = Star();
            double bv = (rand() / (double)RAND_MAX) * 2.4 - 0.4; // -0.4 to 2.0
            bv2rgb(r, g, b, bv);
            s.colorR = r;
            s.colorG = g;
            s.colorB = b;
            s.id = (unsigned int)i;
            s.radius = abs(ran(generator) + 1.0f)+0.1f;
            s.posX = posX + (ran(generator) + 1.0f) * clusterOffsetMean * i;
            s.posY = posY + (ran(generator) + 1.0f) * clusterOffsetMean * i;
            s.posZ = posZ + (ran(generator) + 1.0f) * clusterOffsetMean * i;
            stars->addStar(s);
        }


    }*/


    const int numOfCluster = 10000;
    const int clusterSizeMean = 3;
    const float clusterOffsetMean = 3.0f;

    //global->galaxySeed = std::chrono::system_clock::now().time_since_epoch().count();
    global->galaxySeed = 100;

    std::default_random_engine generator(global->galaxySeed);
    std::normal_distribution<float> ran(0.0, 1.0);

    srand(global->galaxySeed);
    for (int i = 0; i < numOfCluster; i++) {

        vec3 pos = vec3(ran(generator) * 800.0f,
            ran(generator) * 800.0f,
            ran(generator) * 800.0f);
        double r, g, b;

        int clusterSize = ceil(abs(ran(generator) + 1.0f) * clusterSizeMean);

        GalaxyCluster* cluster = new GalaxyCluster(0, clusterSize, pos, galaxyLayer);
        galaxyCluster.push_back(cluster);

        for (int i = 0; i < clusterSize; i++) {
            double bv = (rand() / (double)RAND_MAX) * 2.4 - 0.4; // -0.4 to 2.0
            vec3 gPos = vec3(pos.x + (ran(generator) + 1.0f) * clusterOffsetMean * i,
                pos.y + (ran(generator) + 1.0f) * clusterOffsetMean * i,
                pos.z + (ran(generator) + 1.0f) * clusterOffsetMean * i);
            vec3 gPole = glm::normalize(vec3(rand()-RAND_MAX/2.0f,
                rand()-RAND_MAX/2.0f, rand()-RAND_MAX/2.0f));
            int type = i;
            float radius = abs(ran(generator) + 1.0f) + 0.1f;
            cluster->newGalaxy(gPos, bv, gPole, radius, type);
        }
    }


    logger << "Finished init." << "\n";
    //Loop
    while (!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);

        //float time = (float)glfwGetTime();
        
        /*
        for (int i = 0; i < 10; i++) {
            switch (i%4)
            {
            case 0:
                cubes->addRotation(cubeInstances[i], glm::vec3(1.0f, 0.0f, 0.0f));
                break;
            case 1:
                cubes->addRotation(cubeInstances[i], glm::vec3(0.0f, 1.0f, 0.0f));
                break;
            case 2:
                cubes->addRotation(cubeInstances[i], glm::vec3(0.0f, 0.0f, 1.0f));
                break;
            case 3:
                break;
            }
        }
        whiteLight.ambient.x = (sin(glfwGetTime() * 2.0f) + 1.0f) / 2.0f;
        whiteLight.ambient.y = (sin(glfwGetTime() * 1.9f) + 1.0f) / 2.0f;
        whiteLight.ambient.z = (sin(glfwGetTime() * 1.8f) + 1.0f) / 2.0f;

        whiteLight.diffuse.x = (sin(glfwGetTime() * 1.7f) + 1.0f) / 2.0f;
        whiteLight.diffuse.y = (sin(glfwGetTime() * 1.6f) + 1.0f) / 2.0f;
        whiteLight.diffuse.z = (sin(glfwGetTime() * 1.5f) + 1.0f) / 2.0f;

        whiteLight.specular.x = (sin(glfwGetTime() * 1.4f) + 1.0f) / 2.0f;
        whiteLight.specular.y = (sin(glfwGetTime() * 1.3f) + 1.0f) / 2.0f;
        whiteLight.specular.z = (sin(glfwGetTime() * 1.2f) + 1.0f) / 2.0f;

        whiteLight.power = 1000.0f;

        renderer->light = whiteLight;

        auto pos = camera->getPosition();
        */

        //stars->update();

        galaxyLayer->update();


        //pre render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render

        /*renderer->updateValue();
        renderer->render(cube->getRenderData(), camera->getRenderData());
        renderer->render(cubes->getRenderData(), camera->getRenderData());
        renderer->render(grids->getRenderData(), camera->getRenderData());
        lightRenderer->render(lightCube->getRenderData(), camera->getRenderData());
        


        starsRenderer->updateValue();
        starsRenderer->render(stars->getRenderData());

        printVec3(stars->getSpeed());
        printVec3(stars->getCameraPosition(true));
        */
        galaxyRenderer->preRender();
        galaxyRenderer->render(galaxyLayer->getRenderData());
        galaxyRenderer->postRender();

        //endTick
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}