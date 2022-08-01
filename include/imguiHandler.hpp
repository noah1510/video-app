#pragma once

#include <exception>
#include <stdexcept>
#include <string>
#include <iostream>

#include <glad/glad.h>
#include "imgui.h"

#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"


#if  __has_include("SDL2/SDL.h") && __has_include("SDL2/SDL_thread.h")
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_thread.h>
    #include <SDL_opengl.h>
#else
    #error "cannot include sdl2" 
#endif

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace sakurajin{
    class imguiHandler{
        private:
        std::string glsl_version = "#version 460 core";
        SDL_GLContext gl_context;
        SDL_Window* window = nullptr;
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
            
        imguiHandler();
        ~imguiHandler();
            
        static imguiHandler& getInstance(){
            static imguiHandler instance{};
            return instance;
        }
        
        void init_impl();
        void startRender_impl();
        void endRender_impl();
        void initFramebuffer_impl(unsigned int& FBO, unsigned int& texture, uint64_t width = 3840, uint64_t height = 2160);
        void loadFramebuffer_impl ( unsigned int buf, uint64_t width = 3840, uint64_t height = 2160 );
        void updateRenderThread_impl();
        
        public:
        static void init(){
            getInstance().init_impl();
        }
        
        static void startRender(){
            getInstance().startRender_impl();
        }
        
        static void endRender(){
            getInstance().endRender_impl();
        }
        
        static void initFramebuffer(unsigned int& FBO, unsigned int& texture, uint64_t width = 3840, uint64_t height = 2160){
            getInstance().initFramebuffer_impl(FBO,texture,width,height);
        }
        
        static void loadFramebuffer ( unsigned int buf, uint64_t width = 3840, uint64_t height = 2160 ){
            getInstance().loadFramebuffer_impl(buf, width, height);
        }
        
        static void updateRenderThread(){
            getInstance().updateRenderThread_impl();
        }
        
    };
}
