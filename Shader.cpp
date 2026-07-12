#include "Shader.h"
#include <iostream>
#include "Application.h"
#include "AssetManager.h"

static constexpr int TYPE_VERTEX = 0;
static constexpr int TYPE_FRAGMENT = 1;
static constexpr int TYPE_PROGRAM = 2;

Shader::Shader() {}

// Shader::Shader(Shader&& other) noexcept
//     : m_ShaderID(other.m_ShaderID),
//       m_vertexPath(std::move(other.m_vertexPath)),
//       m_fragmentPath(std::move(other.m_fragmentPath)),
//       m_isCompiled(other.m_isCompiled),
//       m_hasErrors(other.m_hasErrors)
// {
//     other.m_ShaderID = 0; // Steal the ID, prevent old object from deleting it
// }

void Shader::GetVertexFragmentPath(const char* shaderName, std::string& out_vertexPath, std::string& out_fragmentPath) {
    std::string path = Application::ResourcesPath  + "/Shaders/" + std::string(shaderName);
    out_vertexPath = (path + ".vert");
    out_fragmentPath = (path + ".frag");
}

Shader::Shader(const char* shaderName) : m_ShaderID(0), m_isCompiled(false), m_hasErrors(false)
{
    m_name = shaderName;
    SetName(shaderName);
}

void Shader::SetName(const char* shaderName) {
    m_name = shaderName;
    GetVertexFragmentPath(shaderName, m_vertexPath,  m_fragmentPath);
}


Shader::Shader(const char* vertPath, const char* fragPath)
        : m_ShaderID(0),  m_isCompiled(false), m_hasErrors(false)
{
    m_name = vertPath;
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

unsigned int Shader::GetShaderId() const {
    return m_ShaderID;
}

std::string& Shader::GetName() {
    return m_name;
}


int Shader::LoadAndCompile() {
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

    // printf("VERTEX CODE! \n%s\n", vertexCode.c_str());
    // printf("Shader CODE! \n%s\n", fragmentCode.c_str());

    if (!didReadVert) {
        std::cerr << "Failed to load vertex shader! " << m_vertexPath.c_str() << std::endl;
        return 1;
    }
    if (!didReadFrag) {
        std::cerr << "Failed to load fragment shader! " << m_fragmentPath.c_str() << std::endl;
        return 2;
    }
    // printf("--- step 2\n");
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
        m_isCompiled = false;
        glDeleteProgram(m_ShaderID);
        m_ShaderID = 0;
        return 10;
        std::cerr << "Failed error CHECK FOR PROGRAM!" << std::endl;
        return 150;
    }
    // Always detach and delete intermediate shader objects after linking
    glDeleteShader(idVert);
    glDeleteShader(idFrag);
    m_isCompiled = true;
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
        fprintf(stderr, "Shader error found: %s\n", compilerLog.c_str());
        return false;
    }
    return true;
}


// region Basic Properties Set
void Shader::SetBool(const std::string& name, bool value) const {
    int loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniform1i(loc, (int)value);
}

void Shader::SetInt(const std::string& name, int value) const {
    int loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniform1i(loc, value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    int loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniform1f(loc, value);
}
//endregion


// region CGLM
void Shader::SetVec3(const std::string& name, const vec3 v) const {
    int loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0) {
        glUniform3fv(loc, 1, (const float*)v);
    }
}
void Shader::SetVec4(const std::string& name, const vec4 v) const {
    int loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0) {
        glUniform4fv(loc, 1, (const float*)v);
    }
}
void Shader::SetMat4(const std::string& name, const mat4 m) const {
    int loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)m);
}


std::string& Shader::GetVertexPath() {
    return m_vertexPath;
}

std::string& Shader::GetFragmentPath() {
    return m_fragmentPath;
}

// endregion

