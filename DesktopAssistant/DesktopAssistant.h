#pragma once

#include <windows.h>
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
void LoadWaifu(HINSTANCE hinstance);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// NotifyIcon
void InitNotifyIcon(HWND hWnd, HINSTANCE hInstance);
void DeleteNotifyIcon();
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
