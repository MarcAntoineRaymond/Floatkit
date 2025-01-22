#include "DesktopAssistant.h"

NOTIFYICONDATA nidApp;
HMENU hPopMenu;
WNDPROC originalWndProc = DefWindowProc;
bool draggingW = false;
int nCmd = 0;

void InitNotifyIcon(HWND hWnd, HINSTANCE hInstance)
{
    originalWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
    HICON hIcon = (HICON)LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DESKTOPASSISTANT));
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

// Custom Windows Procedure to handle tray menu interactions
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static POINT clickOffset;
    int wmId, wmEvent;
    switch (msg) {
    case WM_USER_SHELLICON:
        // systray msg callback
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
            UINT uFlag = MF_BYPOSITION | MF_STRING;
            GetCursorPos(&clickOffset);
            hPopMenu = CreatePopupMenu();
            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MENU_OPTIONS, L"Options");
            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_SEPARATOR, IDM_SEP, L"SEP");
            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MENU_ABOUT, L"About");
            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MENU_EXIT, L"Exit");

            SetForegroundWindow(hwnd);
            TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, clickOffset.x, clickOffset.y, 0, hwnd, NULL);
            return TRUE;
        }
        break;
    case WM_COMMAND:
        wmId = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case ID_MENU_ABOUT:
            DialogBox((HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, About);
            break;
        case ID_MENU_OPTIONS:
            //OpenOptions();
            // TODO
            break;
        case ID_MENU_EXIT:
            PostQuitMessage(0);
            break;
        default:
            return originalWndProc(hwnd, msg, wParam, lParam);
        }
        break;
    default:
        return originalWndProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}