#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Camera.h>
#include <Shader.h>
#include <Model.h>
#include <OpenGLUtil.h>
#include <Plane.h>
#include <Cube.h>

#define WINDOW_HEIGHT 1080  
#define WINDOW_WIDTH 1920



Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

bool flashlightOn = false;
bool fKeyPressedLastFrame = false;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateDeltaTime();
void processFlashlight(Shader& shader);


//Call back function to resize openGL whenever the window changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}


glm::vec3 pointLightPositions[] = {
    glm::vec3( 0.7f,  0.2f,  2.0f),
};


int main() {

    // Initialize libraries
    glfwInit();
    
    //Set hints for OpenGL version and core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGLExperimentation", NULL, NULL);

    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        // Destroys library
        glfwTerminate();
        return -1;
    }

    //
    glfwMakeContextCurrent(window);

    // Load OpenGL functions through glad 
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
	    std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // calls a the function when the window is resized
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Hide cursors and capture it
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // Set key callback
    glfwSetKeyCallback(window, key_callback);

    glfwSetScrollCallback(window, scroll_callback);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    Shader lightingShader("assets/shaders/backpack.vert", "assets/shaders/backpack.frag");

    
    // Model
    std::string backpackPath = "assets/objects/backpack/backpack.obj";
    Model backpack(std::filesystem::absolute(backpackPath).string());
   

    int planeTextureDiffuse = createTexture("assets/textures/container2.png");
    int planeTextureSpecular = createTexture("assets/textures/container2_specular.png");

    std::vector<Texture> planeTextures = {
        Texture(planeTextureDiffuse, "texture_diffuse", "container2.png"),
        Texture(planeTextureSpecular, "texture_specular", "container2_specular.png")
    };



    Plane groundPlane(glm::vec3(0.0f, -1.0f, 0.0f), 0.0f, planeTextures);
    Cube groundCube(planeTextures);


    //Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    
    glm::mat4 projection;
    glm::mat4 view;


    lightingShader.use();

    // Directional light
    lightingShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    lightingShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    lightingShader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    // Spotlight
    lightingShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    lightingShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    lightingShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("spotLight.constant", 1.0f);
    lightingShader.setFloat("spotLight.linear", 0.09f);
    lightingShader.setFloat("spotLight.quadratic", 0.032f);


    // Point lights
    lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[0].constant", 1.0f);
    lightingShader.setFloat("pointLights[0].linear", 0.09f);
    lightingShader.setFloat("pointLights[0].quadratic", 0.032f);


    // Light angle
    float lightAngle = 0.0f;
    float lightSpeed = 4.0f;
    float lightRadius = 1.0f;

    // RENDER LOOP
    while(!glfwWindowShouldClose(window)) {

        updateDeltaTime();

        //Handle input
        processInput(window);

        //Render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        
        lightingShader.use();
        lightingShader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);

        lightingShader.setVec3("viewPos", camera.Position);


        // Uniforms for the 4 point lights
        // point light 1
        lightAngle += lightSpeed * deltaTime;
        float x = lightRadius * cos(lightAngle);
        float z = lightRadius * sin(lightAngle);
        pointLightPositions[0] = glm::vec3(x, 0.2f, z);
        lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);

        

        // Update Flashlight
        lightingShader.setVec3("spotLight.position", camera.Position);
        lightingShader.setVec3("spotLight.direction", camera.Front);

        processFlashlight(lightingShader);


        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        lightingShader.setMat4("view", view);
        lightingShader.setMat4("projection", projection);


        // Render the backpack model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));	// Scale it down
        lightingShader.setMat4("model", model);
        backpack.Draw(lightingShader);

        // Render ground plane
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
        lightingShader.setMat4("model", model);
        groundPlane.Draw(lightingShader);


        // Render ground cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(5.0f, 0.0f, 0.0f));
        lightingShader.setMat4("model", model);
        groundCube.Draw(lightingShader);

        glBindVertexArray(0);
        // Check call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }


    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);

}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void updateDeltaTime() {
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);


    

}

void processFlashlight(Shader& shader) {
    if (flashlightOn) {
        shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(17.5f)));
    } else {
        shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(0.0f)));
        shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(0.0f)));
    }

}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        flashlightOn = !flashlightOn;
    }
}
