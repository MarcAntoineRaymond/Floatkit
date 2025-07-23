#pragma once
#include <windows.h>
#include <vector>

//#include <GL/GL.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
//#define GLFW_NATIVE_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


//#include <glm/glm.hpp>
#include <fx/gltf.h>
#include <filesystem>
#include <iostream>
#define USE_VRMC_VRM_1_0 // Enable VRM 1.0 support
#include <VRMC/VRM.h>
#include "Handle3d.h"

struct Vertex {
    float position[3];
    float normal[3];
    float texCoord[2];
};

std::vector<Vertex> vertices;
GLuint VAO, VBO;

const std::string& defaultFile = "models\\rem.vrm";

// OpenGL initialization
void InitOpenGL(HWND hwnd) {
    wglCreateContext(GetDC(hwnd));
    LoadVRMModel(defaultFile);
    //RenderScene();
    //SetupMesh();
}

void LoadVRMModel(const std::string& filePath) {
    try {
        std::filesystem::path f{ filePath };
        const auto doc = fx::gltf::LoadFromBinary(f);
        const auto extensions = doc.extensionsAndExtras["extensions"];

        VRMC_VRM_1_0::Vrm vrm;
        VRMC_VRM_1_0::from_json(extensions["VRMC_vrm"], vrm);

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

        std::cout << "VRM Model Loaded: " << vrm.meta.name << std::endl;
        std::cout << "VRM Model Loaded: " << vertices.size() << " vertices extracted." << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error loading VRM model: " << e.what() << std::endl;
    }
}

// OpenGL render function
void RenderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Bind and draw the VRM model
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);

    SwapBuffers(wglGetCurrentDC());  // Swap buffers in the WinAPI window
}

void SetupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}