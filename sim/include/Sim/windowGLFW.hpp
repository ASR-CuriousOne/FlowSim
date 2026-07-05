#pragma once
#include <GLFW/glfw3.h>

struct WindowGLFW {
    GLFWwindow *m_window;

    const uint32_t WIDTH = 1024;
    const uint32_t HEIGHT = 1024;

    bool m_framebufferResized = false;

	float m_scrollOffset = 0.0f;

    WindowGLFW();
    ~WindowGLFW();

    GLFWwindow *getWindow() const { return m_window; }
    operator GLFWwindow *() const { return getWindow(); }

	float consumeScrollOffset();

    bool shouldClose() { return glfwWindowShouldClose(m_window); }

    static void framebufferResizeCallback(GLFWwindow *window, int width,
                                          int height);
	static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

};
