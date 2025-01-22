#pragma once

bool InitOpenGL(HWND hwnd);
void LoadVRMModel(const std::string& filePath);
void PrepareScene();
void RenderScene();
void SetupMesh();
void DestroyScene();
void gl_check_error();