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

#include <ft2build.h>
#include FT_FREETYPE_H

#include "Camera.h"
#include "Shader.h"
#include "Model.h"
#include "OpenGLUtil.h"
#include "Plane.h"
#include "Cube.h"
#include "SkyCube.h"
#include "PointLight.h"
#include "FontManager.h"

#define WINDOW_HEIGHT 1080  
#define WINDOW_WIDTH 1920
#define SHADOW_WIDTH 1024
#define SHADOW_HEIGHT 1024

int fbWidth = WINDOW_WIDTH;
int fbHeight = WINDOW_HEIGHT;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WINDOW_WIDTH / 2.0f, lastY = WINDOW_HEIGHT / 2.0f;
bool firstMouse = true;

bool flashlightOn = false;
bool useDither = false;
bool showShadowMap = false;
bool showDirectionalLightDebug = false;
bool showPointShadowMap = false;
bool useParallaxMapping = true;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// Pure direction vector for directional light (direction light rays travel)
glm::vec3 directionalLightDirection(-0.6f, -0.3f, -0.45f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void processInput(GLFWwindow *window);
void updateDeltaTime();
void processFlashlight(Shader& shader);
void renderScene(Shader& shader, Plane& groundPlane, Cube& cube2, Cube& cube3, Model* model, Model* model2, Model* model3, Model* model4);
void directionalDebugArrow(Shader& lightCubeShader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& direction, Cube& lightCube);
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int textVAO, unsigned int textVBO);

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

    // Query actual framebuffer size (Retina/high-DPI aware) before creating FBO attachments.
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

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
    Shader pointDepthShader("assets/shaders/pointShadowsDepth.vert", "assets/shaders/pointShadowsDepth.frag", "assets/shaders/pointShadowsDepth.geom");
    Shader textShader("assets/shaders/renderText.vert", "assets/shaders/renderText.frag");


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
    screenShader.setFloat("exposure", 0.5f);

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
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB8, fbWidth, fbHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msaaColorRBO);

    // Create multisampled depth-stencil renderbuffer
    GLuint msaaDepthRBO;
    glGenRenderbuffers(1, &msaaDepthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthRBO);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, fbWidth, fbHeight, 0, GL_RGB, GL_FLOAT, NULL);
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


    
    // Omni-directional shadow cubemap
    GLuint shadowCubemapFBO;
    glGenFramebuffers(1, &shadowCubemapFBO);
    
    GLuint shadowCubemap;
    glGenTextures(1, &shadowCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, shadowCubemap);

    for (unsigned int i = 0; i < 6; ++i) {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowCubemapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);



    // Tell shaders which texture unit will hold the shadow map
    depthShader.use();
    depthShader.setInt("shadowMap", 0);

    shader.use();
    shader.setInt("shadowMap", 0);

    
    // Model
    Model* mococoModel = new Model("assets/objects/mococo_abyssgard/scene.gltf"); 
    
    if (!mococoModel) {
        std::cout << "No GLTF model loaded for Mococo." << std::endl;
    }

    // Small Fauna Model
    Model* smallFaunaModel = new Model("assets/objects/smallfauna/scene.gltf");
    if (!smallFaunaModel) {
        std::cout << "No GLTF model loaded for Small Fauna." << std::endl;
    }

    // Nimi Nightmare Model
    Model* nimiModel = new Model("assets/objects/nimi_nightmare/scene.gltf");

    if (!nimiModel) {
        std::cout << "No GLTF model loaded for Nimi Nightmare." << std::endl;
    }
   
    Model* beatrice = new Model("assets/objects/beatrice/scene.gltf");

    if (!beatrice) {
        std::cout << "No GLTF model loaded for Beatrice." << std::endl;
    }

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
    shader.setInt("shadowMap", 0);
    shader.setInt("skybox", 1);  // Use texture unit 1 for cubemap
    shader.setInt("shadowCubeMap", 2); // Use texture unit 2 for point light shadow cubemap


    

    // LIGHT SETUP

    // Directional light
    shader.setVec3("dirLight.direction", glm::normalize(directionalLightDirection));
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


    float aspect = (float)SHADOW_WIDTH / (float)SHADOW_HEIGHT;
    float near = 0.1f;
    float far = 25.0f;

    // Point lights
    PointLight pointLight1(0, glm::vec3(0.0f, 3.5f, 0.0f), glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f), 1.0f, 0.09f, 0.032f, aspect, near, far);
    pointLight1.setShaderUniforms(shader);

    // Height for point light: half the cube height (approx 0.5)
    float pointLightHeight = 0.5f;

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // Light angle
    float lightAngle = 0.0f;
    float lightSpeed = 4.0f;
    float lightRadius = 1.0f;

    // Shadow camera frustum for directional light (must cover scene extents)
    float near_plane = 0.1f, far_plane = 60.0f;
    glm::mat4 lightProjection = glm::ortho(-20.0f, 20.0f, -20.0f, 20.0f, near_plane, far_plane);

    // For directional shadows, derive a virtual light camera position from direction
    glm::vec3 sceneCenter(0.0f, 0.0f, 0.0f);
    glm::vec3 lightDir = glm::normalize(directionalLightDirection);
    float lightDistance = 20.0f;
    glm::vec3 lightVirtualPos = sceneCenter - lightDir * lightDistance;
    glm::vec3 lightUp = (glm::abs(glm::dot(lightDir, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
        ? glm::vec3(0.0f, 0.0f, 1.0f)
        : glm::vec3(0.0f, 1.0f, 0.0f);

    glm::mat4 lightView = glm::lookAt(lightVirtualPos, sceneCenter, lightUp);
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    debugQuadShader.use();
    debugQuadShader.setFloat("near_plane", near_plane);
    debugQuadShader.setFloat("far_plane", far_plane);


    


    // =================================================
    // ================ Font Stuff =====================
    // =================================================
    
    FontManager::Instance().InitFont("assets/fonts/ByteBounce.ttf", 48);
    
    unsigned int textVAO, textVBO;
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6*4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::mat4 orthoProjection = glm::ortho(0.0f, (float)fbWidth, 0.0f, (float)fbHeight);
    textShader.use();
    textShader.setMat4("projection", orthoProjection);

    // ================================================
    // ================ Render Loop =====================
    // ================================================

    int lastFbWidth = fbWidth;
    int lastFbHeight = fbHeight;

    while(!glfwWindowShouldClose(window)) {

        // Keep offscreen attachments in sync with framebuffer size when the window changes.
        if (fbWidth != lastFbWidth || fbHeight != lastFbHeight) {
            glBindRenderbuffer(GL_RENDERBUFFER, msaaColorRBO);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGB8, fbWidth, fbHeight);

            glBindRenderbuffer(GL_RENDERBUFFER, msaaDepthRBO);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, fbWidth, fbHeight);

            glBindTexture(GL_TEXTURE_2D, textureColorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbWidth, fbHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            orthoProjection = glm::ortho(0.0f, (float)fbWidth, 0.0f, (float)fbHeight);
            textShader.use();
            textShader.setMat4("projection", orthoProjection);

            lastFbWidth = fbWidth;
            lastFbHeight = fbHeight;
        }

        updateDeltaTime();

        processInput(window);

        // Update moving lights: move point light in a circle around origin on the XZ plane
        lightAngle += deltaTime * lightSpeed;
        float newX = cos(lightAngle) * lightRadius;
        // float newZ = sin(lightAngle) * lightRadius;
        pointLight1.updatePosition(glm::vec3(newX, pointLightHeight, pointLight1.position.z));

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Shadow pass state: keep back-face culling and use polygon offset to reduce self-shadow acne
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(2.0f, 4.0f);

        depthShader.use();
        depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

        

        // Render scene from light's perspective
        renderScene(depthShader, groundPlane, cube2, cube3, mococoModel, nimiModel, smallFaunaModel, beatrice);
        
        pointDepthShader.use();

        // Update depth shader uniforms after moving light and recomputing shadow transforms
        pointLight1.setDepthShaderUniforms(pointDepthShader);
        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);

        glBindFramebuffer(GL_FRAMEBUFFER, shadowCubemapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        // Render scene from light's perspective
        renderScene(pointDepthShader, groundPlane, cube2, cube3, mococoModel, nimiModel, smallFaunaModel, beatrice);

        glDisable(GL_POLYGON_OFFSET_FILL);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(0, 0, fbWidth, fbHeight);

        // Render to MSAA framebuffer first
        glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO);
        

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // Render the scene
        shader.use();
        shader.setBool("showPointShadowMap", showPointShadowMap);
        shader.setVec3("cameraPosition", camera.Position);
        // Forward parallax toggle to the scene shader so fragments can apply POM
        shader.setBool("useParallaxMapping", useParallaxMapping);

        // Bind cubemap for reflections (reserved unit 1)
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

        // Bind shadow map to texture unit 0 for sampling in the shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, shadowMap);
        shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);


        // Update Flashlight
        shader.setVec3("spotLight.position", camera.Position);
        shader.setVec3("spotLight.direction", camera.Front);

        processFlashlight(shader);


        // view/projection transformations
        projection = glm::perspective(glm::radians(camera.Zoom), (float)fbWidth / (float)fbHeight, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);
        
        // Lighting uniforms that may change per-frame
        pointLight1.setShaderUniforms(shader);
        pointLight1.bindShadowMap(shadowCubemap);


        renderScene(shader, groundPlane, cube2, cube3, mococoModel, nimiModel, smallFaunaModel, beatrice);

        if (showDirectionalLightDebug) {
            directionalDebugArrow(lightCubeShader, view, projection, directionalLightDirection, lightCube);

            // Draw the debug cube for the point light (after view/projection are set)
            lightCubeShader.use();
            lightCubeShader.setMat4("view", view);
            lightCubeShader.setMat4("projection", projection);
            pointLight1.drawDebugCube(lightCubeShader, view, projection);
        }

        


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
        glBlitFramebuffer(0, 0, fbWidth, fbHeight, 0, 0, fbWidth, fbHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        // Bind default framebuffer for post-processing pass
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, fbWidth, fbHeight);

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

        if (useParallaxMapping) {
            screenShader.setBool("useParallaxMapping", true);
        } else {
            screenShader.setBool("useParallaxMapping", false);
        }

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        RenderText(textShader, "I suppose", fbWidth/2.0f, fbHeight/2.0f, 1.0f, glm::vec3(0.5, 0.8f, 0.2f), textVAO, textVBO);

        glEnable(GL_DEPTH_TEST); // Re-enable depth testing for next frame
        // Check call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }


    if (mococoModel) {
        delete mococoModel;
        mococoModel = nullptr;
    }
    if (smallFaunaModel) {
        delete smallFaunaModel;
        smallFaunaModel = nullptr;
    }
    if (nimiModel) {
        delete nimiModel;
        nimiModel = nullptr;
    }
    if (beatrice) {
        delete beatrice;
        beatrice = nullptr;
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
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        showDirectionalLightDebug = !showDirectionalLightDebug;
    }
    if (key == GLFW_KEY_P && action == GLFW_PRESS) {
        showPointShadowMap = !showPointShadowMap;
    }
    if (key == GLFW_KEY_B && action == GLFW_PRESS) {
        useParallaxMapping = !useParallaxMapping;
    }
}

void renderScene(Shader& shader, Plane& groundPlane, Cube& cube2, Cube& cube3, Model* modelPtr, Model* modelPtr2, Model* modelPtr3, Model *modelPtr4) {
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

    if (modelPtr) {
        glm::mat4 mococoModelMat = glm::mat4(1.0f);
        mococoModelMat = glm::translate(mococoModelMat, glm::vec3(0.0f, -0.5f, -2.5f));
        shader.setMat4("model", mococoModelMat);
        modelPtr->Draw(shader);
    }
    if (modelPtr2) {
        // Build a rotation first (model-space), then translate it into world space.
        glm::mat4 rot = glm::mat4(1.0f);
        // Rotate so the model stands upright. Tweak angles if it still lies on its side.
        rot = glm::rotate(rot, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // X-axis
        rot = glm::rotate(rot, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));  // Y-axis flip
        rot = glm::rotate(rot, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));   // Z axis flip to correct orientation

        glm::mat4 nimiModelMat = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.5f, -2.5f)) * rot;
        shader.setMat4("model", nimiModelMat);
        modelPtr2->Draw(shader);
    }
    if (modelPtr3) {
        glm::mat4 smallFaunaModelMat = glm::mat4(1.0f);
        smallFaunaModelMat = glm::translate(smallFaunaModelMat, glm::vec3(-1.0f, -0.5f, -2.5f));
        shader.setMat4("model", smallFaunaModelMat);
        modelPtr3->Draw(shader);
    }

    if (modelPtr4) {
        glm::mat4 rot = glm::mat4(1.0f);
        
        // Model is looking down the positive Z axis in model space, so rotate it to face the camera which looks down negative Z.
        rot = glm::rotate(rot, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 beatriceModelMat = glm::mat4(1.0f);
        beatriceModelMat = glm::translate(beatriceModelMat, glm::vec3(0.0f, -0.5f, 2.5f)) * rot;
        shader.setMat4("model", beatriceModelMat);
        modelPtr4->Draw(shader);
    }
}

void directionalDebugArrow(Shader& lightCubeShader, const glm::mat4& view, const glm::mat4& projection, const glm::vec3& direction, Cube& lightCube) {
    // Single long rectangular prism aligned with directional light direction
    glm::vec3 forward = glm::normalize(direction);
    glm::vec3 up = (glm::abs(glm::dot(forward, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f)
        ? glm::vec3(0.0f, 0.0f, 1.0f)
        : glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    up = glm::normalize(glm::cross(forward, right));

    glm::mat4 rotation(1.0f);
    rotation[0] = glm::vec4(right, 0.0f);
    rotation[1] = glm::vec4(up, 0.0f);
    rotation[2] = glm::vec4(forward, 0.0f);

    glm::vec3 origin(0.0f, 2.0f, 0.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), origin) * rotation;
    model = model * glm::scale(glm::mat4(1.0f), glm::vec3(0.08f, 0.08f, 1.4f));

    lightCubeShader.use();
    lightCubeShader.setMat4("view", view);
    lightCubeShader.setMat4("projection", projection);
    lightCubeShader.setMat4("model", model);
    lightCube.Draw(lightCubeShader);
}

//Call back function to resize openGL whenever the window changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {

    fbWidth = width;
    fbHeight = height;

    glViewport(0, 0, fbWidth, fbHeight);
}

void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int textVAO, unsigned int textVBO) {
	// activate corresponding render state
	shader.use();
	glUniform3f(glGetUniformLocation(shader.ID, "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(textVAO);
	
	// iterate through all characters
	std::string::const_iterator c;
	
	for (c = text.begin(); c != text.end(); c++) {
		
		Character ch = FontManager::Instance().Get(*c);
		float xpos = x + ch.bearing.x * scale;
		float ypos = y - (ch.size.y - ch.bearing.y) * scale;
		float w = ch.size.x * scale;
		float h = ch.size.y * scale;
		
		// update VBO for each character
		float vertices[6][4] = {
			{ xpos, ypos + h, 0.0f, 0.0f },
			{ xpos, ypos, 0.0f, 1.0f },
			{ xpos + w, ypos, 1.0f, 1.0f },
			{ xpos, ypos + h, 0.0f, 0.0f },
			{ xpos + w, ypos, 1.0f, 1.0f },
			{ xpos + w, ypos + h, 1.0f, 0.0f }
		};
		
		// render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		
		// update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, textVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
		// render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		// advance cursors for next glyph (advance is 1/64 pixels)
		x += (ch.advance >> 6) * scale; // bitshift by 6 (2^6 = 64)
	}
	
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}