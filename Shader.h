#pragma once

#include <string>
#include <cglm/cglm.h>


class Shader {
public:
    Shader();

    Shader(const char* shaderName);
    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    
    // Destructor clears the program from GPU memory
    ~Shader();

    void SetName(const char* shaderName);
    static void GetVertexFragmentPath(const char* shaderName, std::string& out_vertexPath, std::string& out_fragmentPath);

    [[nodiscard]] std::string& GetName();

    [[nodiscard]] std::string& GetVertexPath();

    [[nodiscard]] std::string& GetFragmentPath();

    // Activate the shader program
    void Use() const;

    int LoadAndCompile();


    [[nodiscard]] bool IsCompiled() const;
    [[nodiscard]] unsigned int GetShaderId() const;

    // Utility Uniform Setters
    void SetBool(const std::string& name, bool value) const;
    void SetInt(const std::string& name, int value) const;
    void SetFloat(const std::string& name, float value) const;

    // cglm Type Uniform Setters
    void SetVec3(const std::string& name, const vec3 v) const;
    void SetVec4(const std::string& name, const vec4 v) const;
    void SetMat4(const std::string& name, const mat4 m) const;


private:
    // Helper function to check compilation/linking errors
    static bool CheckCompileErrors(unsigned int shaderID, int type);

    std::string m_name;
    std::string m_vertexPath;
    std::string m_fragmentPath;
    bool m_isCompiled;
    bool m_hasErrors;
    unsigned int m_ShaderID;
};