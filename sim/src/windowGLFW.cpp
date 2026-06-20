#include <Sim/windowGLFW.hpp>
#include <iostream>

WindowGLFW::WindowGLFW() {
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

    if (!m_window) {
        const char *description;
        int code = glfwGetError(&description);
        std::cerr << "Failed to create GLFW window! Error Code: " << code
                  << " Description: " << (description ? description : "Unknown")
                  << std::endl;
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    glfwSetScrollCallback(m_window, scrollCallback);
}

WindowGLFW::~WindowGLFW() {
    glfwDestroyWindow(m_window);

    glfwTerminate();
}

float WindowGLFW::consumeScrollOffset() {
    float offset = m_scrollOffset;
    m_scrollOffset = 0.0f;

    return offset;
}

void WindowGLFW::framebufferResizeCallback(GLFWwindow *window, int width,
                                           int height) {
    auto app = reinterpret_cast<WindowGLFW *>(glfwGetWindowUserPointer(window));
    app->m_framebufferResized = true;
}

void WindowGLFW::scrollCallback(GLFWwindow *window, double xoffset,
                                double yoffset) {
    auto app = reinterpret_cast<WindowGLFW *>(glfwGetWindowUserPointer(window));
    app->m_scrollOffset += static_cast<float>(yoffset);
}
