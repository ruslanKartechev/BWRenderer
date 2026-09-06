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
    GetVertexFragmentPath(shaderName, m_mainPath,  m_fragmentPath);
}

void Shader::SetNameAndPathSeparate(const char* shaderName, const char* vertex, const char* fragment) {
    m_name = shaderName;
    m_mainPath = ProjectSettings::ResourcesPath  + "/Shaders/" + std::string(vertex) + ".vert";
    m_fragmentPath = ProjectSettings::ResourcesPath  + "/Shaders/" + std::string(fragment) + ".frag";
}

void Shader::SetNameAndPath(const char* shaderName, const char* path) {
    m_name = shaderName;
    m_mainPath = ProjectSettings::ResourcesPath  + "/Shaders/" + std::string(path);
}



Shader::Shader(const char* vertPath, const char* fragPath)
        : m_ShaderID(0),  m_isCompiled(false), m_hasErrors(false)
{
    m_name = vertPath;
    m_mainPath = (ProjectSettings::ResourcesPath + "/Shaders/" + std::string(vertPath));
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

    std::string vertexCode;
    std::string fragmentCode;
    bool didReadVert = AssetManager::ReadStringContent(vertexCode, m_mainPath.c_str());
    bool didReadFrag = AssetManager::ReadStringContent(fragmentCode, m_fragmentPath.c_str());
    if (!didReadVert) {
        std::cerr << "Failed to load vertex shader! " << m_mainPath.c_str() << std::endl;
        return COMPILE_CODE_FAILED_TO_READ_VERTEX;
    }
    if (!didReadFrag) {
        std::cerr << "Failed to load fragment shader! " << m_fragmentPath.c_str() << std::endl;
        return COMPILE_CODE_FAILED_TO_READ_FRAGMENT;
    }

    u32 code = CompileGLSLCode(newShaderId, vertexCode, fragmentCode);
    if (code != 0) {
        return code;
    }
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
    return code;
}


i32 Shader::CompileCustomShader() {
    std::string content;
    bool didRead = AssetManager::ReadStringContent(content, m_mainPath.c_str());
    if (!didRead) {
        std::cerr << "Failed to load vertex shader! " << m_mainPath.c_str() << std::endl;
        return COMPILE_CODE_FAILED_TO_READ_VERTEX;
    }
    std::string outVert;
    std::string outFrag;
    auto code = ParseShaderToGLSL(outVert, outFrag, content);
    code = CompileGLSLCode(m_ShaderID, outVert, outFrag);
    if (code != 0) {
        std::cerr << "Failed to COMPILE !!!!! " << m_mainPath.c_str() << std::endl;
    }
    return code;
}


i32 Shader::CompileRawGLSL() {
    std::string vertexCode;
    std::string fragmentCode;
    bool didReadVert = AssetManager::ReadStringContent(vertexCode, m_mainPath.c_str());
    bool didReadFrag = AssetManager::ReadStringContent(fragmentCode, m_fragmentPath.c_str());
    if (!didReadVert) {
        std::cerr << "Failed to load vertex shader! " << m_mainPath.c_str() << std::endl;
        return COMPILE_CODE_FAILED_TO_READ_VERTEX;
    }
    if (!didReadFrag) {
        std::cerr << "Failed to load fragment shader! " << m_fragmentPath.c_str() << std::endl;
        return COMPILE_CODE_FAILED_TO_READ_FRAGMENT;
    }
    auto code = CompileGLSLCode(m_ShaderID, vertexCode, fragmentCode);
    return code;
}


static bool FindIndexOfNextDigit(const char* string, imax startIdx, imax stringLen, imax& out_index) {
    for (imax i = startIdx; i < stringLen; i++) {
        if (string[i] >= 48 && string[i] < 58) {
            out_index = i;
            return true;
        }
    }
    return false;
}

static bool FindIndexOfNextChar(char targetChar, const char* string, imax startIdx, imax stringLen, imax& out_index) {
    for (imax i = startIdx; i < stringLen; i++) {
        if (string[i] == targetChar) {
            out_index = i;
            return true;
        }
    }
    return false;
}


static bool CheckWord(const char* string, imax startIdx, imax stringLen, std::string word) {
    imax wordLen = word.length();
    imax maxLen = min(stringLen, startIdx + wordLen);
    imax charPtr = 0;

    for (imax i = startIdx; i < maxLen && charPtr < wordLen; i++) {
        if (std::tolower(string[i]) != std::tolower(word[charPtr])) {
            return false;
        }
        charPtr++;
    }
    return true;
}


static i32 ReadNumber(const char* string, imax startIdx, imax stringLen) {
    std::string letters;
    i32 outNumber = 0;
    i32 countBrakes = 0;
    for (imax i = startIdx; i < stringLen; i++) {
        char pp = string[i];
        if (string[i] == '\n' || string[i] == '\r') {
            countBrakes++;
            break;
        }
        if (string[i] == ' ')
            continue;
        // 0 and 9
        if (string[i] >= 48 && string[i] < 58) {
            letters.append(1, string[i]);
        }
    }
    std::string copy = std::string(letters);
    try {
        outNumber = std::stoi(letters);
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Invalid argument: Not a valid number.\n";
    }
    catch (const std::out_of_range& e) {
        std::cerr << "Out of range: Number too large for an int.\n";
    }
    return outNumber;
}


static imax SkipToNextLine(const char* string, imax startIdx, imax stringLen) {
    imax idx = startIdx;
    while (string[idx] != '\n') {
        idx++;
        if (idx >= stringLen) {
            return startIdx;
        }
    }
    return idx+1;
}




int Shader::ParseShaderToGLSL(std::string& out_vert, std::string& out_frag, const std::string fileContent)
{
    i32 returnCode = 0;
    const imax len = fileContent.size();
    bool foundVertex = false;
    bool foundFragment = false;
    i32 version = 320;
    std::string wordVersion = "version";
    std::string wordSection = "section";
    std::string wordEndSection = "endsection";

    std::string wordVertex = "Vertex";
    std::string wordFragment = "Fragment";
    const char* strPtr = fileContent.c_str();
    out_vert.reserve(len);
    out_frag.reserve(len);


    for(imax i = 0; i < len; i++)
    {
        switch (fileContent[i]) {
            // case '\n':
            //     continue;
            //     break;
            case '#':
                // shader version
                if (CheckWord(strPtr, i+1, len, wordVersion)) {

                    bool didFind = FindIndexOfNextDigit(strPtr, (i+wordVersion.length()), len, i);
                    if (!didFind) {
                        return COMPILE_CODE_NOT_FOUND_VERSION_DIGITS;
                    }
                    version = ReadNumber(strPtr, i, len);
                    i = SkipToNextLine(strPtr, i, len);
                    out_vert = std::string("#version ") + std::to_string(version) + " core\n";
                    out_frag = std::string("#version ") + std::to_string(version) + " core\n";

                }
                // start of the #section
                else if (CheckWord(strPtr, i+1, len, wordSection)) {
                    imax nwStart = i + 1 + wordSection.length();
                    while (nwStart < len && strPtr[nwStart] == ' ') {
                        nwStart++;
                    }

                    char ff = strPtr[nwStart];
                    bool isVertexSection = false;
                    if (CheckWord(strPtr, nwStart, len, wordVertex)) {
                        isVertexSection = true;
                        foundVertex = true;
                    } else if (CheckWord(strPtr, nwStart, len, wordFragment)) {
                        isVertexSection = false;
                        foundFragment = true;
                    }
                    else {
                        printf("ERROR FAILED PARSE SECTION TYPE\n");
                        return COMPILE_CODE_ERROR;
                    }
                    i = SkipToNextLine(strPtr, i, len);

                    imax sectionEndIdx = 0;
                    if (!FindIndexOfNextChar('#', strPtr, i, len, sectionEndIdx)) {
                        return COMPILE_CODE_END_SECTION_NOT_FOUND;
                    }
                    // Skip empty characters
                    while (i < len && (strPtr[i] == '\n' || strPtr[i] == '\r' || strPtr[i] == ' '))
                    {i++;}

                    i32 copyLen = sectionEndIdx - i - 2;
                    if (isVertexSection) {
                        // printf("Found vertex section %d - %d\n", i, sectionEndIdx);
                        out_vert.insert(out_vert.size(), fileContent, i, copyLen);
                    }
                    else {
                        // printf("Found fragment section %d - %d\n", i, sectionEndIdx);
                        out_frag.insert(out_frag.size(), fileContent, i, copyLen);
                    }
                    i = sectionEndIdx;
                }
                // endSection
                else if (CheckWord(strPtr, i+1, len, wordEndSection)) {

                    i = SkipToNextLine(strPtr, i, len);
                }
                break;
        }
    }
    if (!foundVertex) {
        returnCode = COMPILE_CODE_NOT_FOUND_VERTEX;
    }
    if (!foundFragment) {
        returnCode = COMPILE_CODE_NOT_FOUND_FRAGMENT;
    }
    return returnCode;
}



i32 Shader::CompileGLSLCode(u32& newId, const std::string& vertexCode, const std::string& fragmentCode) {
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


i32 Shader::GetUniformLocation(const char* name) {
    return glGetUniformLocation(m_ShaderID, name);
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
        std::cerr << " FAILED TO SET VEC 2 " << m_name << "   " << name << std::endl;
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
    return m_mainPath;
}

std::string& Shader::GetFragmentPath() {
    return m_fragmentPath;
}

// endregion

