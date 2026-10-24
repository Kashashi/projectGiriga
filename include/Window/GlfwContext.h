#ifndef GIRIGARA_WINDOW_GLFW_CONTEXT_H
#define GIRIGARA_WINDOW_GLFW_CONTEXT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdexcept>

class GlfwContext {
public:
    GlfwContext() {
        if (!glfwInit()) {
            throw std::runtime_error("[GlfwContext] Failed to initialize GLFW");
        }

        // 指定現代 OpenGL 4.6 Core Profile
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    ~GlfwContext() {
        glfwTerminate();
    }

    // 單一 GLFW Context 實例，禁止拷貝與移動
    GlfwContext(const GlfwContext&) = delete;
    GlfwContext& operator=(const GlfwContext&) = delete;
    GlfwContext(GlfwContext&&) = delete;
    GlfwContext& operator=(GlfwContext&&) = delete;

    // 當 WindowManager 建立視窗並 makeContextCurrent 後，呼叫此處統一初始化 GLAD 與預設渲染狀態
    static void initGLAD() {
        if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
            throw std::runtime_error("[GlfwContext] Failed to initialize GLAD");
        }

        // 開啟 Alpha 混合
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
};

#endif // GIRIGARA_WINDOW_GLFW_CONTEXT_H
