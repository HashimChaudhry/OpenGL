#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "Shader.h"
#include "stb_image.h"
#include "Camera.h"
#include "Model.h"


#include <iostream>

using namespace std;
using namespace glm;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Camera set-up
Camera camera(vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f; // Time between current frame and last frame;
float lastFrame = 0.0f;

// Lighting set-up
vec3 lightPos(1.2f, 1.0f, 2.0f);

int main() {
    // Initialize GLFW to create a context for OpenGL
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // Set version of OpenGL to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // Set version of OpenGL to 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Set OpenGL to core profile

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT);
#endif // __APPLE__

    // Create a window using GLFW
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Coordinate Systems", NULL, NULL);
    if (window == NULL) { // If window creation failed
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }

    // Set current contexts and callback functions
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Set up GLFW input mode
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD: Load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        cout << "Failed to initialize GLAD" << endl;
        return -1;
    }

    // Flip loaded textures on y-axis
    stbi_set_flip_vertically_on_load(true);

    // Enable depth testing to allow proper drawing
    glEnable(GL_DEPTH_TEST);

    // Build and Compile our shaders
    Shader shader("model.vs", "model.fs");

    // Load models
    Model backpackModel("backpack/backpack.obj");

    // Draw in wireframe
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    // Render Loop
    while (!glfwWindowShouldClose(window)) {
        // Per-frame time logic
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);

        // Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // Activate shader
        shader.use();

        // Set up view/projection transformations
        mat4 projection = perspective(radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        mat4 view = camera.GetViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        // World transformations
        mat4 model = mat4(1.0f);
        model = translate(model, vec3(0.0f, 0.0f, 0.0f));
        model = scale(model, vec3(1.0f, 1.0f, 1.0f));
        shader.setMat4("model", model);
        backpackModel.Draw(shader);

        // Swap buffers and poll I/O events
        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    // Terminate the program
    glfwTerminate();
    return 0;
}


// Process all input by querying GLFW whether relevant kets are pressed/released
// this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Whenever the window size is changed, this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    // Make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displats
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}


