#include <stdio.h>
#include <stdlib.h>
#include "video_reader.hpp"
#include "shader.hpp"

int main(int argc, const char** argv) {
    GLFWwindow* window;

    if (!glfwInit()) {
        printf("Couldn't init GLFW\n");
        return 1;
    }

    window = glfwCreateWindow(800, 480, "Hello World", NULL, NULL);
    if (!window) {
        printf("Couldn't open window\n");
        return 1;
    }

    VideoReaderState vr_state;
    if (!video_reader_open(&vr_state, "data/example_video.mp4")) {
        printf("Couldn't open video file (make sure you set a video file that exists)\n");
        return 1;
    }

    glfwMakeContextCurrent(window);
    
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cerr << "Could not create OpenGL context" << std::endl;
        return -1;
    }
    
    std::shared_ptr<sakurajin::Shader> outputShader;
    try{
        outputShader = std::make_shared<sakurajin::Shader>("data/shader.vert","data/shader.frag");
    }catch(const std::exception& e){
        sakurajin::Helper::print_exception(e);
        return -1;
    }
    
    //assign the samplers to the texture units
    outputShader->setUniform("yTex",0);

    // Generate texture
    GLuint texRGB;
    glGenTextures(1, &texRGB);
    glBindTexture(GL_TEXTURE_2D, texRGB);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    // Allocate frame buffer
    constexpr int ALIGNMENT = 128;
    const int frame_width = vr_state.width;
    const int frame_height = vr_state.height;
    uint8_t* frame_data;
    if (posix_memalign((void**)&frame_data, ALIGNMENT, frame_width * frame_height * 4) != 0) {
        printf("Couldn't allocate frame buffer\n");
        return 1;
    }
    
    //load the vertex buffers to store coordinates
    unsigned int VAO = 0, EBO = 0, VBO = 0;
    float vertices[] = {
        // positions    // texture coords
         16.0f,  9.0f,  1.0f, 0.0f, // top right
         16.0f, -9.0f,  1.0f, 1.0f, // bottom right
        -16.0f, -9.0f,  0.0f, 1.0f, // bottom left
        -16.0f,  9.0f,  0.0f, 0.0f  // top left 
    };
    unsigned int indices[] = {  
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // update the viewport
        int window_width, window_height;
        glfwGetFramebufferSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);
        
        //create and update the projection matrix
        float camMult = 0.9;
        auto orth = glm::ortho(
            -16.0f, 
            camMult * 9.0f * window_width / window_height,
            -9.0f,
            camMult * 16.0f * window_height / window_width,
            -10.0f,
            10.0f
        );
        outputShader->setUniform("transform", orth);

        // Read a new frame and load it into texture
        int64_t pts;
        if (!video_reader_read_frame(&vr_state, frame_data, &pts)) {
            printf("Couldn't load video frame\n");
            return 1;
        }

        double pt_in_seconds = pts * (double)vr_state.time_base.num / (double)vr_state.time_base.den;
        while (pt_in_seconds > glfwGetTime()) {
            glfwWaitEventsTimeout(pt_in_seconds - glfwGetTime());
        }

        //activate the texture and the shader
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texRGB);
        glTexImage2D(
            GL_TEXTURE_2D, 
            0, 
            GL_RGBA, 
            vr_state.width, 
            vr_state.height, 
            0, 
            GL_RGBA, 
            GL_UNSIGNED_BYTE, 
            frame_data
        );
        
        outputShader->use();
        
        //draw the rectangle
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        
        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    video_reader_close(&vr_state);

    return 0;
}

