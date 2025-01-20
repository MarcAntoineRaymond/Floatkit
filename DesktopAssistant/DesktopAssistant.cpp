#include "DesktopAssistant.h"

HBITMAP hBitmap = NULL;
int waifuWidth = 0;
int waifuHeight = 0;
ULONG_PTR gdiplusToken;
HMENU hPopMenu;
BOOL bStopped = FALSE;
HINSTANCE hInst;
int nCmd;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    LoadWaifu(hInstance);

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WAIFU_CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName, WAIFU_APP_NAME, WS_POPUP,
        100, 100, 500, 500, NULL, NULL, hInstance, NULL);

    if (!hwnd) return -1;

    nCmd = nCmdShow;
    UpdateImage(hwnd);
    ShowWindow(hwnd, nCmd);
    InitNotifyIcon(hwnd, hInstance);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    DeleteObject(hBitmap);
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static POINT clickOffset;
    int wmId, wmEvent;

    switch (uMsg) {
    case WM_LBUTTONDOWN:
        clickOffset.x = LOWORD(lParam);
        clickOffset.y = HIWORD(lParam);
        SetCapture(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        if (wParam & MK_LBUTTON) {
            POINT pt;
            GetCursorPos(&pt);
            SetWindowPos(hwnd, HWND_TOPMOST, pt.x - clickOffset.x, pt.y - clickOffset.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;

    case WM_LBUTTONUP:
        ReleaseCapture();
        return 0;
    case WM_USER_SHELLICON:
        // systray msg callback 
        switch (LOWORD(lParam))
        {
        case WM_RBUTTONDOWN:
            UINT uFlag = MF_BYPOSITION | MF_STRING;
            GetCursorPos(&clickOffset);
            hPopMenu = CreatePopupMenu();
            InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MENU_ABOUT, L"About");
            if (bStopped == TRUE)
            {
                uFlag |= MF_GRAYED;
            }
		
            if (bStopped == TRUE)
            {
                InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MENU_START, L"Start");
            }
            else
            {
                InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, ID_MENU_STOP, L"Stop");
            }
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
        case ID_MENU_STOP:
            bStopped = TRUE;
            ShowWindow(hwnd, SW_HIDE);
            break;
        case ID_MENU_START:
            bStopped = FALSE;
            ShowWindow(hwnd, nCmd);
            break;
        case ID_MENU_EXIT:
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
    case WM_DESTROY:
        DeleteObject(hBitmap);
        PostQuitMessage(0);
        DeleteNotifyIcon();
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void UpdateImage(HWND hwnd) {
    if (!hBitmap) return;
    BITMAP bm;
    GetObject(hBitmap, sizeof(bm), &bm);
    SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

    // get the primary monitor's info
    POINT ptZero = { 0 };
    HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorinfo = { 0 };
    monitorinfo.cbSize = sizeof(monitorinfo);
    GetMonitorInfo(hmonPrimary, &monitorinfo);

    // Create the splash screen in the bottom right corner of the primary work area
    const RECT& rcWork = monitorinfo.rcWork;
    POINT ptOrigin;
    ptOrigin.x = rcWork.right - sizeSplash.cx;
    ptOrigin.y = rcWork.bottom - sizeSplash.cy;

    // create a memory DC holding the splash bitmap
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hBitmap);

    // use the source image's alpha channel for blending
    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    // paint the window (in the right location) with the alpha-blended bitmap
    UpdateLayeredWindow(hwnd, hdcScreen, &ptOrigin, &sizeSplash,
        hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

    // delete temporary objects
    SelectObject(hdcMem, hbmpOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

void LoadWaifu(HINSTANCE hinstance)
{
    Gdiplus::Bitmap bitmap(L"Shizuka.png");
    bitmap.GetHBITMAP(NULL, &hBitmap);
    waifuWidth = bitmap.GetWidth();
    waifuHeight = bitmap.GetHeight();
}
