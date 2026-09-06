#pragma once

#include <string>
#include <cglm/cglm.h>
#include "MyTypes.h"


class Shader {
public:
    static const i32 COMPILE_CODE_SUCCESS = 0;
    static const i32 COMPILE_CODE_ERROR = 1;
    static const i32 COMPILE_CODE_END_SECTION_NOT_FOUND = 2;
    static const i32 COMPILE_CODE_NOT_FOUND_VERTEX = 3;
    static const i32 COMPILE_CODE_NOT_FOUND_FRAGMENT = 4;
    static const i32 COMPILE_CODE_NOT_FOUND_VERSION_DIGITS = 5;

    static const i32 COMPILE_CODE_FAILED_TO_READ_VERTEX = 10;
    static const i32 COMPILE_CODE_FAILED_TO_READ_FRAGMENT = 11;

    Shader();

    Shader(const char* shaderName);
    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    
    // Destructor clears the program from GPU memory
    ~Shader();

    void SetName(const char* shaderName, bool pathFromName = true);
    void SetNameAndPathSeparate(const char* shaderName, const char* vertex, const char* fragment);
    void SetNameAndPath(const char* shaderName, const char* path);

    static void GetVertexFragmentPath(const char* shaderName, std::string& out_vertexPath, std::string& out_fragmentPath);


    // Activate the shader program
    void Use() const;

    int Recompile();

    int CompileRawGLSL();

    int CompileCustomShader();

    [[nodiscard]] i32 GetUniformLocation(const char* name);

    [[nodiscard]] std::string& GetName();

    [[nodiscard]] std::string& GetVertexPath();

    [[nodiscard]] std::string& GetFragmentPath();



    [[nodiscard]] u32 GetShaderId() const;
    [[nodiscard]] bool IsCompiled() const;
    [[nodiscard]] bool GetAcceptsLighting() const;
    void SetAcceptsLighting(bool value);

    // Utility Uniform Setters
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;

    // CGLM Type Uniform Setters
    void SetVec2(const std::string& name, const vec2 v) const;
    void SetVec3(const std::string& name, const vec3 v) const;
    void SetVec4(const std::string& name, const vec4 v) const;
    void SetMat4(const std::string& name, const mat4 m) const;

    void SetTexture(const std::string& name, i32 textureID) const;

private:
    // Helper function to check compilation/linking errors
    static bool CheckCompileErrors(unsigned int shaderID, int type);

    static int ParseShaderToGLSL(std::string& out_vert, std::string& out_frag, const std::string fileContent);

    static i32 CompileGLSLCode(u32& newId, const std::string& m_vertexPath, const std::string& m_fragmentPath);


    std::string m_name;
    std::string m_mainPath;
    std::string m_fragmentPath;
    bool m_isCompiled;
    bool m_hasErrors;
    bool m_acceptsLighting;

    u32 m_ShaderID = 0;
};