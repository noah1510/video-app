#include <stdio.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include "video_reader.hpp"
#include "shader.hpp"

using namespace std::literals;
const std::string glsl_version = "#version 460 core";
const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
uint64_t fboWidth = 1920;
uint64_t fboHeight = 1080;

int main(int argc, const char** argv) {
    sakurajin::imguiHandler::init();
    unsigned int FBO = 0, outTexture = 0;
    sakurajin::imguiHandler::initFramebuffer(FBO,outTexture, fboWidth, fboHeight);
    
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
    
    //init the video renderer
    VideoReaderState vr_state;
    if (!video_reader_open(&vr_state, "data/example_video.mp4")) {
        printf("Couldn't open video file (make sure you set a video file that exists)\n");
        return 1;
    }

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
         16.0f,  9.0f,  1.0f, 1.0f, // top right
         16.0f, -9.0f,  1.0f, 0.0f, // bottom right
        -16.0f, -9.0f,  0.0f, 0.0f, // bottom left
        -16.0f,  9.0f,  0.0f, 1.0f  // top left 
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
    
    SDL_Event event;
    
    bool exit = false;
    while (!exit) {
        sakurajin::imguiHandler::startRender();
        
        ImGui::Begin("video out");
            
            //update the viewport and load the fbo
            fboHeight = ImGui::GetWindowHeight();
            fboWidth = ImGui::GetWindowWidth();
            
            auto size = ImGui::GetContentRegionAvail();
            auto max = ImGui::GetWindowContentRegionMax();
            auto min = ImGui::GetWindowContentRegionMin();
            size.x = max.x-min.x;
            size.y = max.y-min.y;
            fboWidth = size.x;
            fboHeight = size.y;
            
            sakurajin::imguiHandler::loadFramebuffer(FBO,fboWidth,fboHeight);
            
            outputShader->use();
            //create and update the projection matrix
            float camMult = 0.8;
            auto orth = glm::ortho(
                -16.0f, 
                camMult * 9.0f * fboWidth / fboHeight,
                -9.0f,
                camMult * 16.0f * fboHeight / fboWidth,
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
            
            //draw the rectangle
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            
            glBindVertexArray(0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            
            ImGui::Image((void*)(intptr_t)outTexture, size);
            
            //show a tooltip and window borders when the window is hovered
            if(ImGui::IsItemHovered()){
                ImGui::BeginTooltip();
                ImGui::Text("pointer = %u", outTexture);
                ImGui::Text("size = %lu x %lu", fboWidth, fboHeight);
                ImGui::EndTooltip();
                
                ImVec2 vMin = ImGui::GetWindowContentRegionMin();
                ImVec2 vMax = ImGui::GetWindowContentRegionMax();

                vMin.x += ImGui::GetWindowPos().x;
                vMin.y += ImGui::GetWindowPos().y;
                vMax.x += ImGui::GetWindowPos().x;
                vMax.y += ImGui::GetWindowPos().y;

                ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
            }
        
        ImGui::End();

        sakurajin::imguiHandler::endRender();
        
        while(SDL_PollEvent(&event)){
            ImGui_ImplSDL2_ProcessEvent(&event);
            
            if(event.type == SDL_QUIT){
                exit = true;
                break;
            } else if(event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE){
                exit = true;
                break;
            }
        }
        /*
        double pt_in_seconds = pts * (double)vr_state.time_base.num / (double)vr_state.time_base.den;
        while (pt_in_seconds > glfwGetTime()) {
            glfwWaitEventsTimeout(pt_in_seconds - glfwGetTime());
        }*/
    }

    video_reader_close(&vr_state);

    return 0;
}

