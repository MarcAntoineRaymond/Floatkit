#pragma once
#define NTDDI_VERSION 0x0A000006 //NTDDI_WIN10_RS5
#define _WIN32_WINNT 0x0A00 // _WIN32_WINNT_WIN10, the _WIN32_WINNT macro must also be defined when defining NTDDI_VERSION
#include <windows.h>
#include <iostream>
#include <shobjidl.h>
#include <shlobj_core.h>
#include "resource.h"
#include "gdiplus.h"


#define MAX_LOADSTRING          100
#define	WM_USER_SHELLICON       WM_USER + 1
#define IDS_APPTOOLTIP          104
#define IDI_NOTIFY              105

#define WAIFU_CLASS_NAME    L"WaifuWindowClass"
#define WAIFU_APP_NAME      L"My Waifu"

// Main
void UpdateImage(HWND hwnd);
void LoadWaifu();
void SelectOptions();
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
std::wstring SearchImage();

// NotifyIcon
void InitNotifyIcon(HWND hWnd, HINSTANCE hInstance);
void DeleteNotifyIcon();
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
