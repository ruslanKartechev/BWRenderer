#pragma once

#include <string>
#include <cglm/cglm.h>


class Shader {
public:


    // Shader(const Shader&) = delete;
    // Shader& operator=(const Shader&) = delete;
    //
    // Shader(Shader&&) noexcept;
    // Shader& operator=(Shader&&) noexcept = default;

    Shader();

    Shader(const char* shaderName);
    // Constructor reads and builds the shader
    Shader(const char* vertexPath, const char* fragmentPath);
    
    // Destructor clears the program from GPU memory
    ~Shader();

    void SetName(const char* shaderName);

    [[nodiscard]] std::string GetName() const;

    // Activate the shader program
    void Use() const;
    int LoadAndCompile();

    [[nodiscard]] bool IsCompiled() const;
    [[nodiscard]] unsigned int GetShaderId() const;

    // Utility Uniform Setters
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    // cglm Type Uniform Setters
    void setVec3(const std::string& name, const vec3 v) const;
    void setVec4(const std::string& name, const vec4 v) const;
    void setMat4(const std::string& name, const mat4 m) const;


private:
    // Helper function to check compilation/linking errors
    static bool CheckCompileErrors(unsigned int shaderID, int type);

    std::string m_vertexPath;
    std::string m_fragmentPath;
    bool m_isCompiled;
    bool m_hasErrors;
    unsigned int m_ShaderID;
};