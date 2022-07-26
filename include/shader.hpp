#pragma once

#include <glad/glad.h>
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>

#include "helper.hpp"

namespace sakurajin{
    class Shader_configuration{
    public:
        std::filesystem::path vertex_shader_location = "";
        std::filesystem::path tessControl_shader_location = "";
        std::filesystem::path tessEvaluation_shader_location = "";
        std::filesystem::path geometry_shader_location = "";
        std::filesystem::path fragment_shader_location = "";
        std::filesystem::path compute_shader_location = "";
        
        Shader_configuration(const std::filesystem::path& vertLoc, const std::filesystem::path& fragLoc);
        Shader_configuration(const Shader_configuration& other);
        Shader_configuration();
        
        bool isValid() const;
        
        bool hasTessControlShader() const;
        bool hasTessEvaluationShader() const;
        bool hasGeometryShader() const;
        bool hasComputeShader() const;
        
    };
    
    class Shader{
    private:
        unsigned int shaderProgram = 0;
        
        Shader_configuration shader_config;
        
        int createShader();
        
        int compileShader(std::string code, unsigned int& shaderLoc, int shaderType);
        
        std::string loadFile(std::filesystem::path location);
    public:
        Shader(const Shader_configuration& config);
        Shader(Shader_configuration config);
        
        Shader(const std::filesystem::path& vertLoc, const std::filesystem::path& fragLoc);
        
        ~Shader();
        
        void use();
        
        void setUniform(const std::string& location, int value);
        void setUniform(const std::string& location, glm::vec1 value);
        void setUniform(const std::string& location, glm::vec2 value);
        void setUniform(const std::string& location, glm::vec3 value);
        void setUniform(const std::string& location, glm::vec4 value);
        void setUniform(const std::string& location, const glm::mat4& value);
        
    };
}
