#include "DesktopAssistant.h"

#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <time.h>

bool running = true;

static const struct
{
    float x, y;
    float r, g, b;
} vertices[3] =
{
    { -0.6f, -0.4f, 1.f, 0.f, 0.f },
    {  0.6f, -0.4f, 0.f, 1.f, 0.f },
    {   0.f,  0.6f, 0.f, 0.f, 1.f }
};

GLuint vertex_buffer, vertex_shader, fragment_shader, program;
GLint mvp_location, vpos_location, vcol_location;
GLFWwindow* Window;

bool dragging = false;
double lastX, lastY;

static const char* vertex_shader_text =
"#version 110\n"
"uniform mat4 MVP;\n"
"attribute vec3 vCol;\n"
"attribute vec2 vPos;\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";

static const char* fragment_shader_text =
"#version 110\n"
"varying vec3 color;\n"
"void main()\n"
"{\n"
"    gl_FragColor = vec4(color, 1.0);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
#ifdef _WIN32
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_RELEASE) {
            dragging = false;
            ReleaseCapture();
        }
        else {
            if (action == GLFW_PRESS && dragging == false) {
                glfwGetCursorPos(window, &lastX, &lastY);
                if (!IsPixelTransparent(lastX, lastY)) { // Only drag if non-transparent
                    dragging = true;
                    SetCapture(glfwGetWin32Window(window));
                }
            }
        }
    }
#endif
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
    if (dragging) {
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        RECT rect;
        GetWindowRect(hwnd, &rect);

        int dx = static_cast<int>(xpos - lastX);
        int dy = static_cast<int>(ypos - lastY);

        SetWindowPos(hwnd, NULL, rect.left + dx, rect.top + dy, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
#endif
    }
}

GLFWwindow* CreateGlfwWindow() {
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);  // transparemt/ layering
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // Remove window border
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);    // topmost
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    // Hide from taskbar (Windows only)
    #ifdef _WIN32
    HWND hwnd = glfwGetWin32Window(window);
    SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE);
    SetLayeredWindowAttributes(hwnd, 0x000000, 0, LWA_COLORKEY);
    #endif
    glfwSetKeyCallback(window, key_callback);
    
    // Set mouse callbacks
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    glfwMakeContextCurrent(window);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return window;
}

// Function to check pixel color at cursor position
bool IsPixelTransparent(int x, int y) {
    int width, height;
    glfwGetFramebufferSize(Window, &width, &height);

    if (x < 0 || y < 0 || x >= width || y >= height)
        return true; // Out of bounds = transparent

    unsigned char pixel[4];
    glReadPixels(x, height - y - 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    return pixel[3] < 10; // If alpha < 10, consider it transparent
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    Window = CreateGlfwWindow();
    HWND hwnd = glfwGetWin32Window(Window);

    InitNotifyIcon(glfwGetWin32Window(Window), hInstance);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    vpos_location = glGetAttribLocation(program, "vPos");
    vcol_location = glGetAttribLocation(program, "vCol");

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)0);
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
        sizeof(vertices[0]), (void*)(sizeof(float) * 2));

    while (!glfwWindowShouldClose(Window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;

        glfwGetFramebufferSize(Window, &width, &height);
        ratio = width / (float)height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);

        mat4x4_identity(m);
        mat4x4_rotate_Z(m, m, (float)glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);

        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*)mvp);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(Window);
        glfwPollEvents();
    }

    glfwDestroyWindow(Window);
    DeleteNotifyIcon();

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void gl_check_error() {
    GLenum error = 0;
    const char* err;
    while (error = glGetError() != GL_NO_ERROR) {
        switch (error) {
        case GL_INVALID_ENUM:
            err = "INVALID ENUM";
            break;
        case GL_INVALID_VALUE:
            err = "INVALID VALUE";
            break;
        case GL_INVALID_OPERATION:
            err = "INVALID OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            err = "OUT OF MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            err = "INVALID FRAMEBUFFER OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            err = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            err = "GL_STACK_UNDERFLOW";
            break;
        }
    }
}