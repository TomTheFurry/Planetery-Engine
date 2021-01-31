#include <iostream>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Global.h"

#include "RenderProgram.h"
#include "RenderProgramA.h"
#include "Camera.h"
#include "ModelBase.h"
#include "ShaderProgram.h"



// Absolute Position
// X: left->right  Y: back->front  Z: down->up

// Relative Position
// X: left->right  Y: back->front  Z: down->up

// Rotation
// X: yaw (XY)  Y: pitch (YZ)  Z: roll (XZ)






float camYaw = -90.0f;
float camPitch = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void APIENTRY glDebugOutput(
    GLenum source, GLenum type,
    uint id, GLenum severity,
    GLsizei length, const char* message,
    const void* userParam)
{
    if (id == 131185 || id == 131204 || id == 131169) return;
    logger << "\n--------OPENGL ERROR--------\n";
    logger << "Error id " << std::to_string(id) << ": " << message << "\n";

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:             logger << "Source: API"; break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   logger << "Source: Window System"; break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER: logger << "Source: Shader Compiler"; break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:     logger << "Source: Third Party"; break;
    case GL_DEBUG_SOURCE_APPLICATION:     logger << "Source: Application"; break;
    case GL_DEBUG_SOURCE_OTHER:           logger << "Source: Other"; break;
    default:
        logger << "Source: Unknown (" << std::to_string(source) << ")"; break;
    }
    logger << "\n";

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               logger << "Type: Error"; break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: logger << "Type: Deprecated Behaviour"; break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  logger << "Type: Undefined Behaviour"; break;
    case GL_DEBUG_TYPE_PORTABILITY:         logger << "Type: Portability"; break;
    case GL_DEBUG_TYPE_PERFORMANCE:         logger << "Type: Performance"; break;
    case GL_DEBUG_TYPE_MARKER:              logger << "Type: Marker"; break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          logger << "Type: Push Group"; break;
    case GL_DEBUG_TYPE_POP_GROUP:           logger << "Type: Pop Group"; break;
    case GL_DEBUG_TYPE_OTHER:               logger << "Type: Other"; break;
    default:
        logger << "Type: Unknown (" << std::to_string(type) << ")"; break;
    }
    logger << "\n";

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:         logger << "Severity: high"; break;
    case GL_DEBUG_SEVERITY_MEDIUM:       logger << "Severity: medium"; break;
    case GL_DEBUG_SEVERITY_LOW:          logger << "Severity: low"; break;
    case GL_DEBUG_SEVERITY_NOTIFICATION: logger << "Severity: notification"; break;
    default:
        logger << "Severity: Unknown (" << std::to_string(severity) << ")"; break;
    }
    logger << "\n\n";

}



void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 0.001f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraSpeed = 0.002f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        global->activeCamera->move(vec3(0.0f, cameraSpeed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        global->activeCamera->move(vec3(0.0f, -cameraSpeed, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        global->activeCamera->move(vec3(-cameraSpeed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        global->activeCamera->move(vec3(cameraSpeed, 0.0f, 0.0f));
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        global->activeCamera->move(vec3(0.0f, 0.0f, cameraSpeed));
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        global->activeCamera->move(vec3(0.0f, 0.0f, -cameraSpeed));
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        global->activeCamera->rotate(vec3(0.0f, 0.0f, 1.0f));
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        global->activeCamera->rotate(vec3(0.0f, 0.0f, -1.0f));
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        global->activeCamera->stop();
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
        global->activeCamera->rotate(vec3(mouseOffset.x, mouseOffset.y, 0.0f));
    }
}



RenderProgram* r;
RenderProgramA* ra;
ModelCube* cube;
ModelCube* cube2;
ModelGrid* grid;
ModelGrid* grid2;
ModelPoints* pStar;
Camera* cam;

int main() {
    stbi_set_flip_vertically_on_load(true);
    glfwInit();
    //glfwWindowHint(GLFW_SAMPLES, 32);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
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
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(glDebugOutput, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

    //Class setup
    ShaderProgram::initClass();


    //Gen Model and Renderer
    logger << "Started to load renderer..." << "\n";
    r = new RenderProgram();
    ra = new RenderProgramA();

    logger << "Started to make models..." << "\n";
    cube = new ModelCube();
    cube2 = new ModelCube();
    grid = new ModelGrid(20, 1.0f, vec3(0.7f));
    grid2 = new ModelGrid(10, 1.0f, vec3(1.0f, 0.0f, 0.0f));
    pStar = new ModelPoints(100000, vec3(0), vec3(10), vec3(1.0));


    logger << "Finished making models." << "\n";
    logger << "Started making instances..." << "\n";
    cam = new Camera(global->fov,vec3(0), vec3(0));
    global->activeCamera = cam;


    logger << "Finished init." << "\n";
    //Loop
    while (!glfwWindowShouldClose(window))
    {
        //input
        processInput(window);

        //update
        cam->update();


        //pre render
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render

        r->updateValue();
        r->renderBox(cube);
        r->updateValue();
        r->render(grid);
        ra->updateValue();
        ra->render(pStar);
        ra->postRender();



        //endTick
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}