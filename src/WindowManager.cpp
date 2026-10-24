#include <Window/WindowManager.h>
#include <iostream>
#include <stdexcept>

static void createOrthoProjection(float width, float height, float* m) {
    for (int i = 0; i < 16; ++i) m[i] = 0.0f;
    m[0]  =  2.0f / width;
    m[5]  = -2.0f / height;
    m[10] = -1.0f;
    m[12] = -1.0f;
    m[13] =  1.0f;
    m[15] =  1.0f;
}

WindowManager::WindowManager(int width, int height, const std::string& title, EventBus& eventBus)
: m_width(width), m_height(height), m_eventBus(eventBus) {

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_width, m_height, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwMakeContextCurrent(m_window);
    glfwSetWindowUserPointer(m_window, this);

    glfwSetCursorPosCallback(m_window, staticCursorPosCallback);
    glfwSetMouseButtonCallback(m_window, staticMouseButtonCallback);
    glfwSetKeyCallback(m_window, staticKeyCallback);
    glfwSetWindowFocusCallback(m_window, staticWindowFocusCallback);
    glfwSetFramebufferSizeCallback(m_window, staticFramebufferSizeCallback);
}

WindowManager::~WindowManager() {
    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

void WindowManager::updateProjection(int width, int height) {
    if (m_shader && width > 0 && height > 0) {
        float proj[16];
        createOrthoProjection(static_cast<float>(width), static_cast<float>(height), proj);
        glProgramUniformMatrix4fv(m_shader->getID(), 0, 1, GL_FALSE, proj);
    }
}

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

void WindowManager::staticWindowFocusCallback(GLFWwindow* window, int focused) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onWindowFocus(focused);
}

void WindowManager::staticFramebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto* self = static_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    if (self) self->onFramebufferSize(width, height);
}

void WindowManager::onWindowFocus(int focused) {
    m_windowFocused = (focused == GLFW_TRUE);
    WindowFocusEvent e(m_windowFocused);
    m_eventBus.publish(e);
}

void WindowManager::onKey(int key, int scancode, int action, int mods) {
    if (!m_windowFocused) return;

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        KeyPressedEvent e(key, scancode, mods);
        m_eventBus.publish(e);
    } else if (action == GLFW_RELEASE) {
        KeyReleasedEvent e(key, scancode, mods);
        m_eventBus.publish(e);
    }
}

void WindowManager::onCursorPos(double xpos, double ypos) {
    int winWidth, winHeight, fbWidth, fbHeight;
    glfwGetWindowSize(m_window, &winWidth, &winHeight);
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    if (winWidth <= 0 || winHeight <= 0) return;

    double actualX = xpos * (static_cast<double>(fbWidth) / winWidth);
    double actualY = ypos * (static_cast<double>(fbHeight) / winHeight);

    MouseMovedEvent e(actualX, actualY);
    m_eventBus.publish(e);
}

void WindowManager::onMouseButton(int button, int action, int mods) {
    double xpos, ypos;
    glfwGetCursorPos(m_window, &xpos, &ypos);

    int winWidth, winHeight, fbWidth, fbHeight;
    glfwGetWindowSize(m_window, &winWidth, &winHeight);
    glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
    if (winWidth <= 0 || winHeight <= 0) return;

    double actualX = xpos * (static_cast<double>(fbWidth) / winWidth);
    double actualY = ypos * (static_cast<double>(fbHeight) / winHeight);

    MouseButtonEvent e(button, action, mods, actualX, actualY);
    m_eventBus.publish(e);
}

void WindowManager::onFramebufferSize(int width, int height) {
    if (width <= 0 || height <= 0) return;
    m_width = width;
    m_height = height;
    glViewport(0, 0, width, height);
    updateProjection(width, height);
}
