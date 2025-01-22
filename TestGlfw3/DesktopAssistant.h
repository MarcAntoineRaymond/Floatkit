#pragma once
#define NTDDI_VERSION 0x0A000006 //NTDDI_WIN10_RS5
#define _WIN32_WINNT 0x0A00 // _WIN32_WINNT_WIN10, the _WIN32_WINNT macro must also be defined when defining NTDDI_VERSION
#include <windows.h>
#define GLFW_EXPOSE_NATIVE_WIN32
//#define GLFW_EXPOSE_NATIVE_WGL
#define GLFW_NATIVE_INCLUDE_NONE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <iostream>
#include <shobjidl.h>
#include <shlobj_core.h>
#include "resource.h"
#define MAX_LOADSTRING          100
#define	WM_USER_SHELLICON       WM_USER + 1
#define IDS_APPTOOLTIP          104
#define IDI_NOTIFY              105

#define WAIFU_CLASS_NAME    L"WaifuWindowClass"
#define WAIFU_APP_NAME      L"My Waifu"

// Main
GLFWwindow* CreateGlfwWindow();
static void error_callback(int error, const char* description);
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void OpenOptions();
std::wstring SearchImage();
void gl_check_error();
bool IsPixelTransparent(int x, int y);

// NotifyIcon
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
void InitNotifyIcon(HWND hWnd, HINSTANCE hInstance);
void DeleteNotifyIcon();
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
