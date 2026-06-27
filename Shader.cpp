#include "Shader.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include "Application.h"
#include "AssetManager.h"

static constexpr int TYPE_VERTEX = 0;
static constexpr int TYPE_FRAGMENT = 1;
static constexpr int TYPE_PROGRAM = 2;


Shader::Shader(const char* shaderName) : m_ShaderID(0), m_isCompiled(false), m_hasErrors(false)
{
    std::string strName = Application::ResourcesPath  + "/Shaders/" + std::string(shaderName);
    m_vertexPath = (strName + ".vert");
    m_fragmentPath = (strName + ".frag");
}


Shader::Shader(const char* vertPath, const char* fragPath)
        : m_ShaderID(0),  m_isCompiled(false), m_hasErrors(false)
{
    m_vertexPath = (Application::ResourcesPath + "/Shaders/" + std::string(vertPath));
    m_fragmentPath = (Application::ResourcesPath + "/Shaders/" + std::string(fragPath));
}

Shader::~Shader() {
    if (m_ShaderID != 0) {
        glDeleteProgram(m_ShaderID);
    }
}

void Shader::Use() const {
    glUseProgram(m_ShaderID);
}


bool Shader::IsCompiled() const {
    return m_isCompiled;
}

unsigned int Shader::GetShaderID() const {
    return m_ShaderID;
}



int Shader::Compile() {
    // Update cached paths
    // Clean up existing program if re-compiling with this instance
    if (m_ShaderID != 0) {
        glDeleteProgram(m_ShaderID);
        m_ShaderID = 0;
    }
    std::string vertexCode;
    std::string fragmentCode;
    bool didReadVert = AssetManager::ReadStringContent(vertexCode, m_vertexPath.c_str());
    bool didReadFrag = AssetManager::ReadStringContent(fragmentCode, m_fragmentPath.c_str());

    //printf("VERTEX CODE! \n%s\n", vertexCode.c_str());
    //printf("Shader CODE! \n%s\n", fragmentCode.c_str());

    if (!didReadVert) {
        std::cerr << "Failed to load vertex shader! " << m_vertexPath.c_str() << std::endl;
        return 1;
    }
    if (!didReadFrag) {
        std::cerr << "Failed to load fragment shader! " << m_fragmentPath.c_str() << std::endl;
        return 2;
    }
    printf("--- step 2\n");
    const char* strVert = vertexCode.c_str();
    const char* strFrag = fragmentCode.c_str();

    // Compile Vertex Shader
    GLint codeLength = vertexCode.length();
    GLuint idVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(idVert, 1, &strVert, &codeLength);
    glCompileShader(idVert);
    if (!CheckCompileErrors(idVert, TYPE_VERTEX)) {
        glDeleteShader(idVert);
        return 3;
    }

    // Compile Fragment Shader
    GLuint idFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(idFrag, 1, &strFrag, nullptr);
    glCompileShader(idFrag);
    if (!CheckCompileErrors(idFrag, TYPE_FRAGMENT)) {
        glDeleteShader(idVert);
        glDeleteShader(idFrag);
        return 4;
    }

    // Link Shader Program
    m_ShaderID = glCreateProgram();
    glAttachShader(m_ShaderID, idVert);
    glAttachShader(m_ShaderID, idFrag);
    glLinkProgram(m_ShaderID);

    bool success = CheckCompileErrors(m_ShaderID, TYPE_PROGRAM);
    if (!success) {
        std::cerr << "Failed error CHECK FOR PROGRAM!" << std::endl;
        return 150;
    }
    // Always detach and delete intermediate shader objects after linking
    glDeleteShader(idVert);
    glDeleteShader(idFrag);

    if (!success) {
        m_isCompiled = false;
        glDeleteProgram(m_ShaderID);
        m_ShaderID = 0;
        return 10;
    }
    m_isCompiled = true;
    std::cout << "compiled everything\n";
    return 0;
}


bool Shader::CheckCompileErrors(GLuint shaderID, int type) {
    int success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (success == GL_FALSE) {
        GLint logLength = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
        std::string compilerLog;
        compilerLog.resize(logLength);
        glGetShaderInfoLog(shaderID, logLength, &logLength, compilerLog.data());
        fprintf(stderr, "Shader error found (len: %d): %s\n", logLength, compilerLog.c_str());
        return false;
    }
    return true;
}


// region Basic Properties Set
void Shader::setBool(const std::string& name, bool value) const { 
    glUniform1i(glGetUniformLocation(m_ShaderID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const { 
    glUniform1i(glGetUniformLocation(m_ShaderID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const { 
    glUniform1f(glGetUniformLocation(m_ShaderID, name.c_str()), value);
}
//endregion



// region cglm
// Handles vec3 (Positions, RGB Colors)
void Shader::setVec3(const std::string& name, const vec3 v) const {
    glUniform3fv(glGetUniformLocation(m_ShaderID, name.c_str()), 1, (const float*)v);
}
// Handles vec4 (Positions, RGBA Colors)
void Shader::setVec4(const std::string& name, const vec4 v) const {
    glUniform4fv(glGetUniformLocation(m_ShaderID, name.c_str()), 1, (const float*)v);
}
// Handles mat4 (Model, View, Projection Matrices)
void Shader::setMat4(const std::string& name, const mat4 m) const { 
    // cglm is natively column-major, matching OpenGL's layout perfectly (GL_FALSE)
    glUniformMatrix4fv(glGetUniformLocation(m_ShaderID, name.c_str()), 1, GL_FALSE, (const float*)m);
}
// endregion

