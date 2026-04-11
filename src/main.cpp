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
#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "OpenGLUtil.h"
#include "Plane.h"
#include "Cube.h"
#include "SkyCube.h"
#include "PointLight.h"

#define WINDOW_HEIGHT 1080  
#define WINDOW_WIDTH 1920
#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024



Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

bool flashlightOn = false;
bool useDither = false;
bool showShadowMap = false;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

glm::vec3 directionalLightPosition(0.0f, 1.2f, -3.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateDeltaTime();
void processFlashlight(Shader& shader);
void renderScene(Shader& shader, Plane& groundPlane, Cube& cube2, Cube& cube3);

//Call back function to resize openGL whenever the window changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}



std::vector<glm::vec3> grassPositions;

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
// positions   // texCoords
-1.0f,  1.0f,  0.0f, 1.0f,
-1.0f, -1.0f,  0.0f, 0.0f,
1.0f, -1.0f,  1.0f, 0.0f,

-1.0f,  1.0f,  0.0f, 1.0f,
1.0f, -1.0f,  1.0f, 0.0f,
1.0f,  1.0f,  1.0f, 1.0f
};

int main() {

    // Initialize libraries
    glfwInit();
    
    //Set hints for OpenGL version and core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Multisampling
    glfwWindowHint(GLFW_SAMPLES, 4);

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

    // Enable multisampling
    glEnable(GL_MULTISAMPLE);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Blend
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Face culling
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    Shader shader("assets/shaders/shadowShader.vert", "assets/shaders/shadowShader.frag");
    Shader screenShader("assets/shaders/framebufferScreen.vert", "assets/shaders/framebufferScreen.frag");
    Shader ditherShader("assets/shaders/framebufferScreen.vert", "assets/shaders/framebufferScreenDither.frag");
    Shader debugQuadShader("assets/shaders/debug_quad.vert", "assets/shaders/debug_quad.frag");
    Shader skyboxShader("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    Shader lightCubeShader("assets/shaders/light_cube.vert", "assets/shaders/light_cube.frag");
    Shader depthShader("assets/shaders/lightDepthShader.vert", "assets/shaders/lightDepthShader.frag");


    // Quad for post processing
    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
   
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
   
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    screenShader.use();
    screenShader.setInt("screenTexture", 0);

    ditherShader.use();
    ditherShader.setInt("screenTexture", 0);

    debugQuadShader.use();
    debugQuadShader.setInt("depthMap", 0);

    // Create a multisampled FBO (msaaFBO) for rendering with MSAA, and a single-sample FBO (framebuffer)
    GLuint msaaFBO;
    glGenFramebuffers(1, &msaaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);

    // Create multisampled color renderbuffer
    GLuint msaaColorRBO;
    glGenRenderbuffers(1, &msaaColorRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRBO);

    // Create multisampled depth-stencil renderbuffer
    GLuint msaaDepthRBO;
    glGenRenderbuffers(1, &msaaDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, msaaDepthRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: MSAA Framebuffer is not complete!" << std::endl;
    }

    // Single-sample framebuffer for post-processing (texture attachment)
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // Create a color attachment texture
    GLuint textureColorBuffer;
    glGenTextures(1, &textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorBuffer, 0);

    // Unbind
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Shadow mapping setup
    GLuint shadowMapFBO;
    glGenFramebuffers(1, &shadowMapFBO);
    GLuint shadowMap;
    glGenTextures(1, &shadowMap);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "ERROR: Shadow Map Framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // Tell shaders which texture unit will hold the shadow map
    depthShader.use();
    depthShader.setInt("shadowMap", 0);

    shader.use();
    shader.setInt("shadowMap", 0);

    // Model
    std::string backpackPath = "assets/objects/backpack/backpack.obj";
    Model backpack(std::filesystem::absolute(backpackPath).string());
   

    int planeTextureDiffuse = createTexture("assets/textures/marble.jpg");

    std::vector<Texture> planeTextures = {
        Texture(planeTextureDiffuse, "texture_diffuse", "marble.jpg"),
    };



    Plane groundPlane(glm::vec3(0.0f, 1.0f, 0.0f), 0.0f, planeTextures, 5.0f);

    int cubeTextureDiffuse = createTexture("assets/textures/container2.png");
    int cubeTextureSpecular = createTexture("assets/textures/container2_specular.png");

    std::vector<Texture> cubeTextures = {
        Texture(cubeTextureDiffuse, "texture_diffuse", "container2.png"),
        Texture(cubeTextureSpecular, "texture_specular", "container2_specular.png"),
    };

    Cube groundCube(cubeTextures);


    Cube cube2(cubeTextures);
    Cube cube3(cubeTextures);

    // Small cube used to visualize the point light position
    std::vector<Texture> __emptyTextures;
    Cube lightCube(__emptyTextures);



    // Skybox
    std::vector<std::string> faces
    {
        "assets/textures/skybox/right.jpg",
        "assets/textures/skybox/left.jpg",
        "assets/textures/skybox/top.jpg",
        "assets/textures/skybox/bottom.jpg",
        "assets/textures/skybox/front.jpg",
        "assets/textures/skybox/back.jpg"
    };

    //     std::vector<std::string> faces
    // {
    //     "assets/textures/Hiyoribeer2.png",
    //     "assets/textures/Hiyoribeer2.png",
    //     "assets/textures/Hiyoribeer2.png",
    //     "assets/textures/Hiyoribeer2.png",
    //     "assets/textures/Hiyoribeer2.png",
    //     "assets/textures/Hiyoribeer2.png"
    // };

    GLuint cubemapTexture = createCubeMapTexture(faces);
    std::cout << "Cubemap texture ID: " << cubemapTexture << std::endl;

    SkyCube skybox(cubemapTexture);

    //Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    
    glm::mat4 projection;
    glm::mat4 view;


    shader.use();
    shader.setInt("skybox", 1);  // Use texture unit 1 for cubemap
    shader.setInt("shadowMap", 0);

    // GRASS SETUP
    grassPositions.push_back(glm::vec3(-1.5f, 0.5f, -0.48f));
    grassPositions.push_back(glm::vec3( 1.5f, 0.5f, 0.51f));
    grassPositions.push_back(glm::vec3( 0.0f, 0.5f, 0.7f));
    grassPositions.push_back(glm::vec3(-0.3f, 0.5f, -2.3f));
    grassPositions.push_back(glm::vec3( 0.5f, 0.5f, -0.6f));

    int grassTextureDiffuse = createTexture("assets/textures/grass.png", false);
    std::vector<Texture> grassTextures = {
        Texture(grassTextureDiffuse, "texture_diffuse", "grass.png"),
    };

    Plane grassPlane(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, grassTextures, 1.0f);
    

    // LIGHT SETUP

    // Directional light
    shader.setVec3("dirLight.direction", directionalLightPosition);
    shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);

    // Spotlight
    shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);


    // Point lights
    PointLight pointLight1(glm::vec3(0.0f, 3.5f, 0.0f), glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f);
    pointLight1.setShaderUniforms(shader, 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Light angle
    float lightAngle = 0.0f;
    float lightSpeed = 4.0f;
    float lightRadius = 1.0f;

    float near_plane = 0.1f, far_plane = 7.5f;
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    glm::mat4 lightView = glm::lookAt(directionalLightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    debugQuadShader.use();
    debugQuadShader.setFloat("near_plane", near_plane);
    debugQuadShader.setFloat("far_plane", far_plane);

    

    // RENDER LOOP
    while(!glfwWindowShouldClose(window)) {

        updateDeltaTime();

        //Handle input
        processInput(window);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST); // Enable because its disabled for rendering the quad
        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        // Render scene from light's perspective
        renderScene(depthShader, groundPlane, cube2, cube3);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        // Render to MSAA framebuffer first
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Render the scene
        shader.use();
        shader.setVec3("objectColor", 1.0f, 1.0f, 1.0f);

        shader.setVec3("cameraPosition", camera.Position);

    // Bind cubemap for reflections (reserved unit 1)
    glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    // Bind shadow map to texture unit 0 for sampling in the shader
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);


        // Uniforms for the 4 point lights
        // point light 1
        // lightAngle += lightSpeed * deltaTime;
        // float x = lightRadius * cos(lightAngle);
        // float z = lightRadius * sin(lightAngle);
        // pointLight1.position = glm::vec3(x, 3.5f, z);
        // shader.setVec3("pointLights[0].position", pointLight1.position);
        // Draw a small blue cube at the point light position for 
        

        // Update Flashlight
        shader.setVec3("spotLight.position", camera.Position);
        shader.setVec3("spotLight.direction", camera.Front);

        processFlashlight(shader);


        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        


        // Render ground plane
        renderScene(shader, groundPlane, cube2, cube3);

    // Draw a small debug cube at the directional light pseudo-position
    lightCubeShader.use();
    lightCubeShader.setMat4("view", view);
    lightCubeShader.setMat4("projection", projection);

    glm::mat4 lightDebugModel = glm::mat4(1.0f);
    lightDebugModel = glm::translate(lightDebugModel, directionalLightPosition);
    lightDebugModel = glm::scale(lightDebugModel, glm::vec3(0.2f));
    lightCubeShader.setMat4("model", lightDebugModel);
    lightCube.Draw(lightCubeShader);

        
        // pointLight1.drawDebugCube(lightCubeShader, view, projection);

    
        // Draw skybox last
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        skybox.Draw(skyboxShader);

        glBindVertexArray(0);

        // Resolve MSAA by blitting from msaaFBO to the single-sample framebuffer (framebuffer)
        glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, framebuffer);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glBlitFramebuffer(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Bind default framebuffer for post-processing pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_DEPTH_TEST); // Disable depth test so screen-space quad isn't discarded due to depth test.
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        if (showShadowMap) {
            debugQuadShader.use();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, shadowMap);
        } else {
            if (useDither) {
                ditherShader.use();
            } else {
                screenShader.use();
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
        }

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


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
    if (key == GLFW_KEY_V && action == GLFW_PRESS) {
        useDither = !useDither;
    }
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        showShadowMap = !showShadowMap;
    }
}

void renderScene(Shader& shader, Plane& groundPlane, Cube& cube2, Cube& cube3) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
    shader.setMat4("model", model);
    groundPlane.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));
    shader.setMat4("model", model);
    cube2.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-2.0f, 0.0f, 0.0f));
    shader.setMat4("model", model);
    cube3.Draw(shader);

}
