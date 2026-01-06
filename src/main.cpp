#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <sstream>
#include <fstream>
#include <string>
#include <iostream>
#include <filesystem>
#include "OpenGLUtil.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define WINDOW_HEIGHT 1080  
#define WINDOW_WIDTH 1920


//Call back function to resize openGL whenever the window changes
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

float vertices[] = {
    // positions          // texture coords

    // Back face
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // 0
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // 1
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 2
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // 3

    // Front face
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 4
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // 5
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // 6
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // 7

    // Left face
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 8
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 9
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // 10
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 11

    // Right face
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 12
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 13
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // 14
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 15

    // Bottom face
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // 16
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // 17
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // 18
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // 19

    // Top face
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // 20
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // 21
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // 22
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f  // 23
};

unsigned int indices[] = {
     0,  1,  2,  2,  3,  0,   // back
     4,  5,  6,  6,  7,  4,   // front
     8,  9, 10, 10, 11,  8,   // left
    12, 13, 14, 14, 15, 12,   // right
    16, 17, 18, 18, 19, 16,   // bottom
    20, 21, 22, 22, 23, 20    // top
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnable(GL_DEPTH_TEST);

    GLuint vertexShader = createShader("assets/shaders/triangle.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = createShader("assets/shaders/triangle.frag", GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    // Check success
    {
        int success;
        char infoLog[512];
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if(!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            
        }
    }


    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    

    // Create VAO
    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);



    // Creating and binding the VBO
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // Copy data to the GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Creating and binding EBO
    GLuint EBO;
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


    // Vertex
    //   3    2
    //|-----|--| 
    // x y z s t

    // Tell OpenGL how to interpet the vec3 through layout 0 (aka handle mapping of vbo to shader vertex attributes)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);





    //Wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    

    // Load Create Textures

    GLuint texture1, texture2;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int image_width, image_height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    auto path = std::filesystem::absolute("assets/textures/container.jpg");
    unsigned char *image_data = stbi_load(path.string().c_str(), &image_width, &image_height, &nrChannels, 0);

    if (!image_data) {
        std::cerr << "Failed to load texture: " << stbi_failure_reason() << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "Image dimensions: " << image_width << "x" << image_height << std::endl;


    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, image_width, image_height, 0, format, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(image_data);

    // Image 2
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);
    path = std::filesystem::absolute("assets/textures/awesomeface.png");
    image_data = stbi_load(path.string().c_str(), &image_width, &image_height, &nrChannels, 0);
    std::cout << "Texture2 dimensions: " << image_width << "x" << image_height << ", Channels: " << nrChannels << std::endl;
    if (!image_data) {
        std::cerr << "Failed to load texture: " << stbi_failure_reason() << std::endl;
        glfwTerminate();
        return -1;
    }

    GLenum format2 = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format2, image_width, image_height, 0, format2, GL_UNSIGNED_BYTE, image_data);
    glGenerateMipmap(GL_TEXTURE_2D);


    stbi_image_free(image_data);

    glUseProgram(shaderProgram);

    // Tell OpenGL which texture we want to set first
    glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
    glUniform1i(glGetUniformLocation(shaderProgram, "texture2"), 1);


    // Getting the location of the transform uniform variable
    

    // Model Matrix
    GLuint modelLoc = glGetUniformLocation(shaderProgram,"model");
    // model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));  // Rotate on x axis

    // View Matrix
    GLuint viewLoc = glGetUniformLocation(shaderProgram,"view");
    glm::mat4 view = glm::mat4(1.0f);
    // Translating the scene forwards (-z axis)
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));


    //Perspective Projection Matrix
    GLuint projectionLoc = glGetUniformLocation(shaderProgram,"projection");
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), (float)WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);


    while(!glfwWindowShouldClose(window)) {
        //Handle input
        processInput(window);

        // Transformation matrices to shader

        // Slow rotation
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f) , glm::vec3(0.5f, 1.0f, 0.0f));


        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        //Render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // bind Texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        // render container
        glBindVertexArray(VAO);
        // glDrawArrays(GL_TRIANGLES, 0, 36);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
        
        // Check call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    // Deallocate
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);


    glfwTerminate();
    return 0;
}