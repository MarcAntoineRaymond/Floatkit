#include "Floatkit.h"

NOTIFYICONDATA nidApp;

void InitNotifyIcon(HWND hWnd, HINSTANCE hInstance)
{
    HICON hIcon = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FLOATKIT));
    nidApp.cbSize = sizeof(NOTIFYICONDATA);
    nidApp.hWnd = (HWND)hWnd;
    nidApp.uID = IDI_NOTIFY;
    nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nidApp.hIcon = hIcon;
    nidApp.uCallbackMessage = WM_USER_SHELLICON;
    LoadString(hInstance, IDS_APPTOOLTIP, nidApp.szTip, MAX_LOADSTRING);
    Shell_NotifyIcon(NIM_ADD, &nidApp);
}

void DeleteNotifyIcon()
{
    Shell_NotifyIcon(NIM_DELETE, &nidApp);
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}