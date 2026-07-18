#include "Shader.h"
#include <iostream>
#include "AssetManager.h"
#include "Engine.h"
#include "ProjectDefines.h"
static constexpr i32 TYPE_VERTEX = 0;
static constexpr i32 TYPE_FRAGMENT = 1;
static constexpr i32 TYPE_PROGRAM = 2;

Shader::Shader() : m_isCompiled(false), m_hasErrors(false), m_ShaderID(0) {
}

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
    std::string path = ProjectSettings::ResourcesPath  + "/Shaders/" + std::string(shaderName);
    out_vertexPath = (path + ".vert");
    out_fragmentPath = (path + ".frag");
}

Shader::Shader(const char* shaderName) : m_ShaderID(0), m_isCompiled(false), m_hasErrors(false)
{
    m_name = shaderName;
    SetName(shaderName);
}

void Shader::SetName(const char* shaderName, bool pathFromName) {
    m_name = shaderName;
    GetVertexFragmentPath(shaderName, m_vertexPath,  m_fragmentPath);
}

void Shader::SetNameSeparate(const char* shaderName, const char* vertex, const char* fragment) {
    m_name = shaderName;
    m_vertexPath = ProjectSettings::ResourcesPath  + "/Shaders/" + std::string(vertex) + ".vert";
    m_fragmentPath = ProjectSettings::ResourcesPath  + "/Shaders/" + std::string(fragment) + ".frag";
}



Shader::Shader(const char* vertPath, const char* fragPath)
        : m_ShaderID(0),  m_isCompiled(false), m_hasErrors(false)
{
    m_name = vertPath;
    m_vertexPath = (ProjectSettings::ResourcesPath + "/Shaders/" + std::string(vertPath));
    m_fragmentPath = (ProjectSettings::ResourcesPath + "/Shaders/" + std::string(fragPath));
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

u32 Shader::GetShaderId() const {
    return m_ShaderID;
}

std::string& Shader::GetName(){
    return m_name;
}


i32 Shader::Recompile() {
    u32 newShaderId = 0;
    u32 code = ReadAndCompile(newShaderId, m_vertexPath, m_fragmentPath);

    if (code == 0 ) {
        if (newShaderId != 0) {

#ifdef LOG_SHADER_RECOMPILATION
            std::cout << std::endl << "Compile success! updated shaderID: " << newShaderId  << std::endl;
            std::cout << "Previous ID " << m_ShaderID << " New Shader ID: " << newShaderId << std::endl << std::endl;
#endif
            if (m_ShaderID != newShaderId) {
                glDeleteProgram(m_ShaderID);
            }
            m_ShaderID = newShaderId;
        }
        else {
            std::cerr << "Compile success, but newShaderID is 0!" << std::endl;
        }
    }
    return code;
}


i32 Shader::LoadAndCompile() {
    auto code = ReadAndCompile(m_ShaderID, m_vertexPath, m_fragmentPath);
    return code;
}


i32 Shader::ReadAndCompile(u32& newId, const std::string& m_vertexPath, const std::string& m_fragmentPath) {

    std::string vertexCode;
    std::string fragmentCode;
    bool didReadVert = AssetManager::ReadStringContent(vertexCode, m_vertexPath.c_str());
    bool didReadFrag = AssetManager::ReadStringContent(fragmentCode, m_fragmentPath.c_str());

    if (!didReadVert) {
        std::cerr << "Failed to load vertex shader! " << m_vertexPath.c_str() << std::endl;
        return 1;
    }
    if (!didReadFrag) {
        std::cerr << "Failed to load fragment shader! " << m_fragmentPath.c_str() << std::endl;
        return 2;
    }
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

    newId = glCreateProgram();
    glAttachShader(newId, idVert);
    glAttachShader(newId, idFrag);
    glLinkProgram(newId);

    bool success = CheckCompileErrors(newId, TYPE_PROGRAM);
    if (!success) {
        glDeleteProgram(newId);
        newId = 0;
        return 10;
    }
    // Always detach and delete intermediate shader objects after linking
    glDeleteShader(idVert);
    glDeleteShader(idFrag);
    return 0;
}




bool Shader::CheckCompileErrors(GLuint shaderID, i32 type) {
    i32 success;
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
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniform1i(loc, (i32)value);
}

void Shader::SetInt(const std::string& name, i32 value) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniform1i(loc, value);
}

void Shader::SetFloat(const std::string& name, float value) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniform1f(loc, value);
    // else
        // std::cerr << "Failed to find Location: " << name << std::endl;
}
//endregion


// region CGLM
void Shader::SetVec2(const std::string& name, const vec2 v) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0) {
        glUniform2fv(loc, 1, (const float*)v);
    }
    else {
        std::cerr << " FAILED TO SET VEC 2 " << std::endl;
    }
}

void Shader::SetVec3(const std::string& name, const vec3 v) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0) {
        glUniform3fv(loc, 1, (const float*)v);
    }
}

void Shader::SetVec4(const std::string& name, const vec4 v) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0) {
        glUniform4fv(loc, 1, (const float*)v);
    }
}

void Shader::SetMat4(const std::string& name, const mat4 m) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    if (loc >= 0)
        glUniformMatrix4fv(loc, 1, GL_FALSE, (const float*)m);
}


void Shader::SetTexture(const std::string& name, const i32 textureID) const {
    i32 loc = glGetUniformLocation(m_ShaderID, name.c_str());
    glUniform1i(loc, textureID);
}

void Shader::SetAcceptsLighting(bool value) {
    m_acceptsLighting = value;
}


bool Shader::GetAcceptsLighting() const {
    return m_acceptsLighting;
}


std::string& Shader::GetVertexPath() {
    return m_vertexPath;
}

std::string& Shader::GetFragmentPath() {
    return m_fragmentPath;
}

// endregion

