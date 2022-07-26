#include "shader.hpp"

sakurajin::Shader_configuration::Shader_configuration() {}

sakurajin::Shader_configuration::Shader_configuration(const std::filesystem::path& vertLoc, const std::filesystem::path& fragLoc):
    vertex_shader_location{vertLoc},
    fragment_shader_location{fragLoc}
    {}
    
sakurajin::Shader_configuration::Shader_configuration ( const sakurajin::Shader_configuration& other ) {
    vertex_shader_location = other.vertex_shader_location;
    fragment_shader_location = other.fragment_shader_location;
    tessControl_shader_location = other.tessControl_shader_location;
    tessEvaluation_shader_location = other.tessEvaluation_shader_location;
    geometry_shader_location = other.geometry_shader_location;
    compute_shader_location = other.compute_shader_location;
}

bool sakurajin::Shader_configuration::hasTessControlShader() const {
    return !tessControl_shader_location.empty() && std::filesystem::exists(tessControl_shader_location);
}

bool sakurajin::Shader_configuration::hasTessEvaluationShader() const {
    return !tessEvaluation_shader_location.empty() && std::filesystem::exists(tessEvaluation_shader_location);
}

bool sakurajin::Shader_configuration::hasGeometryShader() const {
    return !geometry_shader_location.empty() && std::filesystem::exists(geometry_shader_location);
}

bool sakurajin::Shader_configuration::hasComputeShader() const {
    return !compute_shader_location.empty() && std::filesystem::exists(compute_shader_location);
}
    
bool sakurajin::Shader_configuration::isValid() const {
    namespace fs = std::filesystem;
    
    //check if required shader parts exist
    if(
        ! fs::exists(vertex_shader_location) ||
        ! fs::exists(fragment_shader_location)
    ){
        return false;
    }
    
    //check if geometry shader exists if it is set
    if(
        ! geometry_shader_location.empty() &&
        ! fs::exists(geometry_shader_location)
    ){
        return false;
    }
    
    //check if geometry shader exists if it is set
    if(
        ! tessControl_shader_location.empty() &&
        ! fs::exists(tessControl_shader_location)
    ){
        return false;
    }
    
    //check if geometry shader exists if it is set
    if(
        ! tessEvaluation_shader_location.empty() &&
        ! fs::exists(tessEvaluation_shader_location)
    ){
        return false;
    }
    
    //check if geometry shader exists if it is set
    if(
        ! compute_shader_location.empty() &&
        ! fs::exists(compute_shader_location)
    ){
        return false;
    }
    
    return true;
    
}

    
sakurajin::Shader::Shader ( const sakurajin::Shader_configuration& config ) : shader_config{config} {
    if(!shader_config.isValid()){
        throw std::invalid_argument("Shder configuration is not valid!");
    }
    
    try{
        createShader();
    }catch(...){
        std::throw_with_nested( std::runtime_error("could not create Shader") );
    }
    
}

sakurajin::Shader::Shader ( sakurajin::Shader_configuration config ) : shader_config{config} {
    if(!shader_config.isValid()){
        throw std::invalid_argument("Shder configuration is not valid!");
    }
    
    try{
        createShader();
    }catch(...){
        std::throw_with_nested( std::runtime_error("could not create Shader") );
    }
}

sakurajin::Shader::Shader ( const std::filesystem::path& vertLoc, const std::filesystem::path& fragLoc ) : shader_config{vertLoc, fragLoc} {
    if(!shader_config.isValid()){
        throw std::invalid_argument("Shder configuration is not valid!");
    }
    
    try{
        createShader();
    }catch(...){
        std::throw_with_nested( std::runtime_error("could not create Shader") );
    }
}


sakurajin::Shader::~Shader() {
    glDeleteProgram(shaderProgram);
}


void sakurajin::Shader::use() {
    glUseProgram(shaderProgram);
}

void sakurajin::Shader::setUniform ( const std::string& location, int value ) {
    auto loc = glGetUniformLocation(shaderProgram, location.c_str() );
    glUniform1i(loc, value);
}

void sakurajin::Shader::setUniform ( const std::string& location, glm::vec1 value ) {
    auto loc = glGetUniformLocation(shaderProgram, location.c_str() );
    glUniform1f(loc, value.x);
}

void sakurajin::Shader::setUniform ( const std::string& location, glm::vec2 value ) {
    auto loc = glGetUniformLocation(shaderProgram, location.c_str() );
    glUniform2f(loc, value.x, value.y);
}

void sakurajin::Shader::setUniform ( const std::string& location, glm::vec3 value ) {
    auto loc = glGetUniformLocation(shaderProgram, location.c_str() );
    glUniform3f(loc, value.x, value.y, value.z);
}

void sakurajin::Shader::setUniform ( const std::string& location, glm::vec4 value ) {
    auto loc = glGetUniformLocation(shaderProgram, location.c_str() );
    glUniform4f(loc, value.x, value.y, value.z, value.w);
}

void sakurajin::Shader::setUniform ( const std::string& location, const glm::mat4& value ) {
    auto loc = glGetUniformLocation(shaderProgram, location.c_str() );
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(value));
}


int sakurajin::Shader::createShader () {
    std::string shaderCode;
    
    //compile and load the vertex shader
    unsigned int vertexShader = 0;
    try{
        shaderCode = loadFile( shader_config.vertex_shader_location );
        compileShader(shaderCode, vertexShader, GL_VERTEX_SHADER);
    }catch(...){
        std::throw_with_nested(std::runtime_error("Could not compile or load vertex shader"));
    }
    
    //compile and load the fragment shader
    unsigned int fragmentShader = 0;
    try{
        shaderCode = loadFile( shader_config.fragment_shader_location );
        compileShader(shaderCode, fragmentShader, GL_FRAGMENT_SHADER);
    }catch(...){
        std::throw_with_nested(std::runtime_error("Could not compile or load fragment shader"));
    }
    
    //optional shader parts
    unsigned int geometryShader = 0, computeShader = 0, tessControlShader = 0, tessEvalShader = 0;
    
    //compile and load the geomerty shader
    if(shader_config.hasGeometryShader()){
        try{
            shaderCode = loadFile( shader_config.geometry_shader_location );
            compileShader(shaderCode, geometryShader, GL_GEOMETRY_SHADER);
        }catch(...){
            std::throw_with_nested(std::runtime_error("Could not compile or load geomerty shader"));
        }
    }
    
    //compile and load the compute shader
    if(shader_config.hasComputeShader()){
        try{
            shaderCode = loadFile( shader_config.compute_shader_location );
            compileShader(shaderCode, computeShader, GL_COMPUTE_SHADER);
        }catch(...){
            std::throw_with_nested(std::runtime_error("Could not compile or load compute shader"));
        }
    }
    
    //compile and load the tessalation control shader
    if(shader_config.hasTessControlShader()){
        try{
            shaderCode = loadFile( shader_config.tessControl_shader_location );
            compileShader(shaderCode, tessControlShader, GL_TESS_CONTROL_SHADER);
        }catch(...){
            std::throw_with_nested(std::runtime_error("Could not compile or load tessalation control shader"));
        }
    }
    
    //compile and load the tessalation evalutaion shader
    if(shader_config.hasTessEvaluationShader()){
        try{
            shaderCode = loadFile( shader_config.tessEvaluation_shader_location );
            compileShader(shaderCode, tessEvalShader, GL_TESS_EVALUATION_SHADER);
        }catch(...){
            std::throw_with_nested(std::runtime_error("Could not compile or load tessalation evalutaion shader"));
        }
    }
        //prepare the shader program to link every part
    int success;
    
    if(shaderProgram){
        glDeleteProgram(shaderProgram);
    }
    shaderProgram = glCreateProgram();
    
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    if(geometryShader){glAttachShader(shaderProgram, geometryShader);};
    if(computeShader){glAttachShader(shaderProgram, computeShader);};
    if(tessControlShader){glAttachShader(shaderProgram, tessControlShader);};
    if(tessControlShader){glAttachShader(shaderProgram, tessControlShader);};
    
    //link the individual shaders to a shader program
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteShader(geometryShader);
        glDeleteShader(computeShader);
        glDeleteShader(tessControlShader);
        glDeleteShader(tessEvalShader);
        
        shaderProgram = 0;
        throw std::runtime_error(std::string{"could not link shader program: "}.append(infoLog));
    }
    
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(geometryShader);
    glDeleteShader(computeShader);
    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvalShader);
    
    return 0;
}

std::string sakurajin::Shader::loadFile ( std::filesystem::path location ) {
    std::string shaderCode;
    std::ifstream ShaderFile;
    // ensure ifstream objects can throw exceptions:
    ShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        // open files
        ShaderFile.open(location);
        std::stringstream ShaderStream;
        // read file's buffer contents into streams
        ShaderStream << ShaderFile.rdbuf();
        // close file handlers
        ShaderFile.close();
        // convert stream into string
        shaderCode = ShaderStream.str();
    } catch (...) {
        std::throw_with_nested(std::runtime_error("Shader file not sucessfully read"));
    }
    
    return shaderCode;
}

int sakurajin::Shader::compileShader( std::string code, unsigned int& shaderLoc, int shaderType ) {
    // compile the shader and return the location of the result
    int success;

    auto shaderCode = code.c_str();

    auto shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, NULL);
    glCompileShader(shader);

    // print compile errors if anything went wrong
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        throw std::runtime_error(std::string{"could not compile shader: "}.append(shaderCode).append(";reason: ").append(infoLog));
    };

    shaderLoc = shader;
    return 0;
}

