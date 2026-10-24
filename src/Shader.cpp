#include <Shader/Shader.h>

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

// 從檔案路徑建構：讀取檔案字串後委託給既有管線
Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode = readFile(vertexPath);
    std::string fragmentCode = readFile(fragmentPath);

    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexCode.c_str());
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vertexShader);
    glAttachShader(m_programID, fragmentShader);
    glLinkProgram(m_programID);

    // 鏈結檢查
    GLint success;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_programID, 512, nullptr, infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(m_programID);
        throw std::runtime_error(std::string("Shader Program Link Failed:\n") + infoLog);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {
    if (m_programID != 0) {
        glDeleteProgram(m_programID);
        m_programID = 0;
    }
}

Shader::Shader(Shader&& other) noexcept : m_programID(other.m_programID) {
    other.m_programID = 0;
}

Shader& Shader::operator=(Shader&& other) noexcept {
    if (this != &other) {
        if (m_programID != 0) {
            glDeleteProgram(m_programID);
        }
        m_programID = other.m_programID;
        other.m_programID = 0;
    }
    return *this;
}

void Shader::use() const {
    glUseProgram(m_programID);
}

// 依名稱設定 Uniform
void Shader::setMat4(const std::string& name, const float* value) const {
    glUniformMatrix4fv(glGetUniformLocation(m_programID, name.c_str()), 1, GL_FALSE, value);
}

void Shader::setVec4(const std::string& name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(m_programID, name.c_str()), x, y, z, w);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_programID, name.c_str()), value);
}

// 依 Location 直接設定 Uniform (使用 glProgramUniform*，不需先行綁定 program)
void Shader::setMat4(GLint location, const float* value) const {
    glProgramUniformMatrix4fv(m_programID, location, 1, GL_FALSE, value);
}

void Shader::setVec4(GLint location, float x, float y, float z, float w) const {
    glProgramUniform4f(m_programID, location, x, y, z, w);
}

void Shader::setInt(GLint location, int value) const {
    glProgramUniform1i(m_programID, location, value);
}

std::string Shader::readFile(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Shader: Failed to open file at path: " + filePath);
    }
    std::stringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

GLuint Shader::compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        glDeleteShader(shader);
        throw std::runtime_error(std::string("Shader Compile Failed:\n") + infoLog);
    }
    return shader;
}
