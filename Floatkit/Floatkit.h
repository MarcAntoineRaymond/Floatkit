#pragma once
#define NTDDI_VERSION 0x0A000006 //NTDDI_WIN10_RS5
#define _WIN32_WINNT 0x0A00 // _WIN32_WINNT_WIN10, the _WIN32_WINNT macro must also be defined when defining NTDDI_VERSION
#include <windows.h>
#include <iostream>
#include <sstream>
#include <shobjidl.h>
#include <shlobj_core.h>
#include "resource.h"
#include "gdiplus.h"

#define MAX_LOADSTRING          100
#define	WM_USER_SHELLICON       WM_USER + 1
#define IDS_APPTOOLTIP          104
#define IDI_NOTIFY              105

#define FLOATKIT_CLASS_NAME    L"FloatkitWindowClass"
#define FLOATKIT_APP_NAME      L"Floatkit"

// Main
// Window
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateImage(HWND hwnd);
void InvalidateCursor();

// Config
std::wstring SearchConfig();
void SelectOptions(HWND hWnd);

// NotifyIcon
void InitNotifyIcon(HWND hWnd, HINSTANCE hInstance);
void DeleteNotifyIcon();
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
