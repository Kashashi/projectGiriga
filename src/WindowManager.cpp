#include <Window/WindowManager.h>
#include <Event/Event.h>
#include <stdexcept>
#include <iostream>

WindowManager::WindowManager(int width, int height, const std::string& title, EventBus& eventBus)
: m_width(width), m_height(height), m_eventBus(eventBus) {

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        throw std::runtime_error("[WindowManager] Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);

    // 將 this 指針綁定至視窗使用者資料
    glfwSetWindowUserPointer(m_window, this);

    // 註冊 GLFW 回調函式
    glfwSetCursorPosCallback(m_window, staticCursorPosCallback);
    glfwSetMouseButtonCallback(m_window, staticMouseButtonCallback);
    glfwSetKeyCallback(m_window, staticKeyCallback);
    glfwSetCharCallback(m_window, staticCharCallback);
    glfwSetWindowFocusCallback(m_window, staticWindowFocusCallback);
    glfwSetFramebufferSizeCallback(m_window, staticFramebufferSizeCallback);
    glfwSetScrollCallback(m_window, staticScrollCallback);
}

WindowManager::~WindowManager() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void WindowManager::updateProjection(int width, int height) {
    m_width = width;
    m_height = height;
    if (m_shader && width > 0 && height > 0) {
        m_shader->use();

        // 原生 OpenGL 欄優先 (Column-major) 4x4 正交矩陣 (左上為 0,0，右下為 width,height)
        float ortho[16] = {
            2.0f / static_cast<float>(width),  0.0f,                              0.0f,  0.0f,
            0.0f,                             -2.0f / static_cast<float>(height), 0.0f,  0.0f,
            0.0f,                              0.0f,                             -1.0f,  0.0f,
            -1.0f,                              1.0f,                              0.0f,  1.0f
        };

        m_shader->setMat4("projection", ortho);
    }
}

// -------------------------------------------------------------
// 成員事件處理 (統一透過 EventBus 發布事件)
// -------------------------------------------------------------

void WindowManager::onCursorPos(double xpos, double ypos) {
    m_eventBus.publish(MouseMovedEvent(xpos, ypos));
}

void WindowManager::onMouseButton(int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);
    m_eventBus.publish(MouseButtonEvent(button, action, mods, xpos, ypos));
}

void WindowManager::onKey(int key, int scancode, int action, int mods) {
    // 1. 發布通用鍵盤事件 (供 UIConsole 優先攔截 Enter、Backspace 等)
    KeyEvent keyEvent(key, scancode, action, mods);
    m_eventBus.publish(keyEvent);

    // 2. 若未被 Console 攔截，且為按下/放開，可依需求向下分發具體語意事件
    if (!keyEvent.isHandled) {
        if (action == GLFW_PRESS) {
            m_eventBus.publish(KeyPressedEvent(key, scancode, mods));
        } else if (action == GLFW_RELEASE) {
            m_eventBus.publish(KeyReleasedEvent(key, scancode, mods));
        }
    }
}

void WindowManager::onChar(unsigned int codepoint) {
    // 發布字元輸入事件 (包含中文字 Unicode Codepoint)
    m_eventBus.publish(CharInputEvent(codepoint));
}

void WindowManager::onWindowFocus(int focused) {
    m_windowFocused = (focused == GLFW_TRUE);
    m_eventBus.publish(WindowFocusEvent(m_windowFocused));
}

void WindowManager::onFramebufferSize(int width, int height) {
    glViewport(0, 0, width, height);
    updateProjection(width, height);

    // ★ 發布視窗大小改變事件
    m_eventBus.publish(WindowResizeEvent(width, height));
}

// -------------------------------------------------------------
// GLFW 靜態回調函式轉發
// -------------------------------------------------------------

void WindowManager::staticCursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onCursorPos(xpos, ypos);
}

void WindowManager::staticMouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onMouseButton(button, action, mods);
}

void WindowManager::staticKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onKey(key, scancode, action, mods);
}

void WindowManager::staticCharCallback(GLFWwindow* window, unsigned int codepoint) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onChar(codepoint);
}

void WindowManager::staticWindowFocusCallback(GLFWwindow* window, int focused) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onWindowFocus(focused);
}

void WindowManager::staticFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onFramebufferSize(width, height);
}

void WindowManager::onScroll(double xoffset, double yoffset) {
    m_eventBus.publish(MouseScrolledEvent(xoffset, yoffset));
}

void WindowManager::staticScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onScroll(xoffset, yoffset);
}
