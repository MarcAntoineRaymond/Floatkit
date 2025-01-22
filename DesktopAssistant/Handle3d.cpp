#pragma once
#include <windows.h>
#include <vector>

#define GLEW_STATIC
#include "GL/glew.h"
#include "GL/wglew.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

//#include <glm/glm.hpp>
#include <fx/gltf.h>
#include <filesystem>
#include <iostream>
//#define USE_VRMC_VRM_1_0 // Enable VRM 1.0 support
#define USE_VRMC_VRM_0_0 // Enable VRM 0.0 support
#include <VRMC/VRM.h>
#include "Handle3d.h"

struct Vertex {
    float position[3];
    float normal[3];
    float texCoord[2];
};

std::vector<Vertex> vertices;
GLuint VAO, VBO;
GLuint textureID = 0;
HGLRC	 m_hrc; // OpenGL Rendering Context
GLuint ShaderProgram;	               // Program
//GLuint &vertexShader;		       // Vertex shader
//GLuint &fragmentShader;		       // Fragment shader

const std::string& defaultFile = "models\\rem.vrm";
void LoadVRMTexture(const fx::gltf::Document& doc);

// OpenGL initialization
bool InitOpenGL(HWND hwnd) {

    PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1 };
    HDC m_dc = GetDC(hwnd);
    if (!m_dc || !GetPixelFormat(m_dc)) {
        pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_SUPPORT_COMPOSITION | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.cAlphaBits = 8;
        pfd.iLayerType = PFD_MAIN_PLANE;
        int format_index = ChoosePixelFormat(m_dc, &pfd);
        if (!format_index)
            return false;

        if (!SetPixelFormat(m_dc, format_index, &pfd))
            return false;
    }

    int active_format_index = GetPixelFormat(m_dc);
    if (!active_format_index) {
        DWORD err = GetLastError();
        std::string message = std::system_category().message(err);
        return false;
    }

    if (!DescribePixelFormat(m_dc, active_format_index, sizeof pfd, &pfd))
        return false;

    if ((pfd.dwFlags & PFD_SUPPORT_OPENGL) != PFD_SUPPORT_OPENGL)
        return false;

    HGLRC tempContext = wglCreateContext(m_dc);
    if (tempContext == NULL) {
        DWORD err = GetLastError();
        std::string message = std::system_category().message(err);
    }
 
    if (!wglMakeCurrent(m_dc, tempContext)) {
        DWORD err = GetLastError();
        std::string message = std::system_category().message(err);
    }
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        const GLubyte* errStr = glewGetErrorString(err);
        return false;
    }

    int attribs[] =
    {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 1,
        WGL_CONTEXT_FLAGS_ARB, 0,
        0
    };

    if (wglewIsSupported("WGL_ARB_create_context") == 1)
    {
        m_hrc = wglCreateContextAttribsARB(m_dc, 0, attribs);
        wglMakeCurrent(NULL, NULL);
        wglDeleteContext(tempContext);
        wglMakeCurrent(m_dc, m_hrc);
    }
    else
    {	//It's not possible to make a GL 3.x context. Use the old style context (GL 2.1 and before)
        m_hrc = tempContext;
    }
    glViewport(0, 0, 2000, 1000);
    return true;
}

void LoadVRMModel(const std::string& filePath) {
    try {
        std::filesystem::path f{ filePath };
        const auto doc = fx::gltf::LoadFromBinary(f);
        const auto extensions = doc.extensionsAndExtras["extensions"];

        VRMC_VRM_0_0::Vrm vrm;
        VRMC_VRM_0_0::from_json(extensions["VRM"], vrm);
        //VRMC_VRM_1_0::Vrm vrm;
        //VRMC_VRM_1_0::from_json(extensions["VRMC_vrm"], vrm);

        for (const auto& mesh : doc.meshes) {
            for (const auto& primitive : mesh.primitives) {
                const auto& posAccessor = doc.accessors[primitive.attributes.find("POSITION")->second];
                const auto& posBufferView = doc.bufferViews[posAccessor.bufferView];
                const auto& posBuffer = doc.buffers[posBufferView.buffer];

                const float* posData = reinterpret_cast<const float*>(posBuffer.data.data() + posBufferView.byteOffset);
                for (size_t i = 0; i < posAccessor.count; ++i) {
                    Vertex vertex = {};
                    vertex.position[0] = posData[i * 3 + 0];
                    vertex.position[1] = posData[i * 3 + 1];
                    vertex.position[2] = posData[i * 3 + 2];

                    vertices.push_back(vertex);
                }
            }
        }

        LoadVRMTexture(doc);
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading VRM model: " << e.what() << std::endl;
    }
}

void LoadVRMTexture(const fx::gltf::Document& doc) {
    if (doc.images.empty()) {
        std::cerr << "No textures found in VRM model." << std::endl;
        return;
    }

    const auto& image = doc.images[0];
    const auto& bufferView = doc.bufferViews[image.bufferView];
    const auto& buffer = doc.buffers[bufferView.buffer];

    int width, height, channels;
    unsigned char* imageData = stbi_load_from_memory(
        buffer.data.data() + bufferView.byteOffset, bufferView.byteLength,
        &width, &height, &channels, 4);

    if (!imageData) {
        std::cerr << "Failed to load VRM texture." << std::endl;
        return;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(imageData);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void PrepareScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLuint shaderProgram = 0;
    GLuint vertexShader = 0;
    GLuint fragmentShader = 0;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    int vlen;
    int flen;
    //minimal.vert
    std::string vertexShaderString = 
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main(void)\n"
        "{\n"
        //"   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "   gl_Position.z = 0.5;\n"
        "   gl_Position.w = 1.0;\n"
        "   TexCoord = aTexCoord;\n"
        "}\n";
    //minimal.frag
    std::string fragmentShaderString =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D ourTexture;\n"
        "void main() {\n"
        "   FragColor = texture(ourTexture, TexCoord);\n"
        "   FragColor = vec4(1.0);\n"
        " }\n";
    vlen = vertexShaderString.length();
    flen = fragmentShaderString.length();
    const char* vertexShaderCStr = vertexShaderString.c_str();
    const char* fragmentShaderCStr = fragmentShaderString.c_str();
    glShaderSource(vertexShader, 1, (const GLchar**)&vertexShaderCStr, &vlen);
    glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentShaderCStr, &flen);

    GLint compiled;

    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compiled);
    if (compiled == FALSE)
    {
        return;
    }

    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compiled);
    if (compiled == FALSE)
    {
        return;
    }

    ShaderProgram = glCreateProgram();

    glAttachShader(ShaderProgram, vertexShader);
    glAttachShader(ShaderProgram, fragmentShader);

    glLinkProgram(ShaderProgram);

    GLint IsLinked;
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, (GLint*)&IsLinked);
    if (IsLinked == FALSE)
    {
        return;
    }
}

// OpenGL render function
void RenderScene() {
    PrepareScene();
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(ShaderProgram);

    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureID);
    //glUniform1i(glGetUniformLocation(ShaderProgram, "ourTexture"), 0);

    glBindVertexArray(VAO);
    glVertexAttrib3f((GLuint)0, 1.0, 0.0, 0.0); // set constant color attribute
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
    gl_check_error();
    SwapBuffers(wglGetCurrentDC());  // Swap buffers in the WinAPI window
}

void SetupMesh() {
    LoadVRMModel(defaultFile);
    //PrepareScene();
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(offsetof(Vertex, texCoord)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    //glEnable(GL_DEPTH_TEST);
}

void DestroyScene()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    //glDeleteBuffers(vertices.size() * sizeof(Vertex), &VBO);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VAO);

    wglMakeCurrent(NULL, NULL);
    if (m_hrc)
    {
        wglDeleteContext(m_hrc);
        m_hrc = NULL;
    }
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