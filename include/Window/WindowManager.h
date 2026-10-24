#ifndef GIRIGARA_WINDOW_WINDOW_MANAGER_H
#define GIRIGARA_WINDOW_WINDOW_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

#include <Shader/Shader.h>
#include <Event/EventBus.h>

class WindowManager {
public:
    WindowManager(int width, int height, const std::string& title, EventBus& eventBus);
    ~WindowManager();

    WindowManager(const WindowManager&) = delete;
    WindowManager& operator=(const WindowManager&) = delete;

    GLFWwindow* getWindow() const { return m_window; }
    bool shouldClose() const { return glfwWindowShouldClose(m_window); }
    void swapBuffers() const { glfwSwapBuffers(m_window); }
    void pollEvents() const { glfwPollEvents(); }

    void setShader(Shader* shader) { m_shader = shader; }
    bool isWindowFocused() const { return m_windowFocused; }

    void updateProjection(int width, int height);

private:
    GLFWwindow* m_window = nullptr;
    int m_width;
    int m_height;
    bool m_windowFocused = true;

    Shader* m_shader = nullptr;
    EventBus& m_eventBus;

    void onCursorPos(double xpos, double ypos);
    void onMouseButton(int button, int action, int mods);
    void onKey(int key, int scancode, int action, int mods);
    void onChar(unsigned int codepoint); // ★ 新增：字元輸入成員函式
    void onWindowFocus(int focused);
    void onFramebufferSize(int width, int height);

    static void staticCursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void staticMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void staticCharCallback(GLFWwindow* window, unsigned int codepoint); // ★ 新增：GLFW 靜態字元回調
    static void staticWindowFocusCallback(GLFWwindow* window, int focused);
    static void staticFramebufferSizeCallback(GLFWwindow* window, int width, int height);
    void onScroll(double xoffset, double yoffset);
    static void staticScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif // GIRIGARA_WINDOW_WINDOW_MANAGER_H
