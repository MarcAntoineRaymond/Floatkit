#include "Floatkit.h"
#include "Animate.h"

std::wstring defaultConfig = L"assets/cat/cat.cfg";

Animate* animateO = nullptr;

ULONG_PTR gdiplusToken;
HMENU hPopMenu;
BOOL bStopped = FALSE;
HINSTANCE hInst;
int nCmd;

POINT dragOffset = { 0, 0 };
bool hasClicked = false;
float g_scale = 1.0f;
ULONGLONG lastFrameTime = GetTickCount64();
float fps = 60.0f;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    animateO = new Animate(defaultConfig);
	if (!animateO->GetLastError().empty()) {
		MessageBox(NULL, animateO->GetLastError().c_str(), L"Error", MB_OK | MB_ICONERROR);
		delete animateO;
		return -1;
	}

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = FLOATKIT_CLASS_NAME;
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowExW(
        WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        wc.lpszClassName, FLOATKIT_APP_NAME, WS_POPUP,
        100, 100, 500, 500, NULL, NULL, hInstance, NULL);

    if (!hwnd) return -1;

    nCmd = nCmdShow;
    UpdateImage(hwnd);
    ShowWindow(hwnd, nCmd);
    InitNotifyIcon(hwnd, hInstance);
    SetTimer(hwnd, 1, static_cast<int>(1000.0f / fps), NULL);

    MSG msg = {};
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    Gdiplus::GdiplusShutdown(gdiplusToken);
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static POINT clickOffset;
    int wmId, wmEvent;

    switch (uMsg) {
    case WM_LBUTTONDOWN:
        SetCapture(hwnd);
        hasClicked = true;
        clickOffset.x = LOWORD(lParam);
        clickOffset.y = HIWORD(lParam);
        return 0;

    case WM_MOUSEMOVE:
        if (hasClicked || (*animateO).IsDragging()) {
            InvalidateCursor();
			// Start dragging
			(*animateO).StartDragging();
            POINT pt;
            GetCursorPos(&pt);
			int pos_x = pt.x - clickOffset.x;
			int pos_y = pt.y - clickOffset.y;
			(*animateO).SetPosition(pos_x, pos_y);
            SetWindowPos(hwnd, HWND_TOPMOST, pos_x, pos_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    case WM_MOUSEWHEEL:
        if ((*animateO).IsDragging())
        {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            float newScale = g_scale + (delta > 0 ? animateO->GetScaleStep() : -animateO->GetScaleStep());
            if (newScale != g_scale) {
                g_scale = newScale;
            }
        }
        return 0;
    case WM_LBUTTONUP:
        ReleaseCapture();
        if (hasClicked && !(*animateO).IsDragging())
            (*animateO).StartClicking();
        (*animateO).StopDragging();
		hasClicked = false;
        InvalidateCursor();
        return 0;
    case WM_USER_SHELLICON:
        // systray msg callback 
        switch (LOWORD(lParam))
        {
		case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
            UINT uFlag = MF_BYPOSITION | MF_STRING;
            GetCursorPos(&clickOffset);
            hPopMenu = CreatePopupMenu();
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
        case ID_MENU_STOP:
            bStopped = TRUE;
            ShowWindow(hwnd, SW_HIDE);
            break;
        case ID_MENU_START:
            bStopped = FALSE;
            ShowWindow(hwnd, nCmd);
            break;
        case ID_MENU_OPTIONS:
            SelectOptions(hwnd);
            break;
        case ID_MENU_EXIT:
            DestroyWindow(hwnd);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        }
        break;
    case WM_SETCURSOR:
        if (LOWORD(lParam) == HTCLIENT) // only override if mouse is in client area
        {
            if ((*animateO).IsDragging())
                SetCursor(LoadCursor(NULL, IDC_SIZEALL)); // dragging cursor
            else
                SetCursor(LoadCursor(NULL, IDC_HAND));   // normal cursor

            return TRUE;
        }
        break;
	case WM_TIMER:
		if (wParam == 1) {
		    UpdateImage(hwnd);
		}
		return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        DeleteNotifyIcon();
        return 0;
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void InvalidateCursor() {
    // Force Windows to re-query the cursor by simulating WM_SETCURSOR
    POINT pt;
    GetCursorPos(&pt);
    HWND hwndUnderCursor = WindowFromPoint(pt);
    if (hwndUnderCursor)
        SendMessage(hwndUnderCursor, WM_SETCURSOR, (WPARAM)hwndUnderCursor, HTCLIENT);
}

void UpdateImage(HWND hwnd) {
    ULONGLONG now = GetTickCount64();
    if (now - lastFrameTime >= 1000.0f / animateO->GetStateFps()) {
        animateO->SetCurrentFrame((animateO->GetCurrentFrame() + 1) % animateO->GetStateCount());
        lastFrameTime = now;
    }
    HBITMAP hImage = (*animateO).GetImage(animateO->GetCurrentFrame());
    if (!hImage) return;

    // Ensure scale if between bounds
    if (g_scale < animateO->GetScaleMin()) g_scale = animateO->GetScaleMin();
    if (g_scale > animateO->GetScaleMax()) g_scale = animateO->GetScaleMax();

    BITMAP bm;
    GetObject(hImage, sizeof(bm), &bm);
    SIZE sizeSplash = {
        static_cast<LONG>(bm.bmWidth * g_scale),
        static_cast<LONG>(bm.bmHeight * g_scale)
    };

    POINT ptOrigin;
    ptOrigin.x = (*animateO).GetPosX();
    ptOrigin.y = (*animateO).GetPosY();

    // create a memory DC holding the splash bitmap
    HDC hdcScreen = GetDC(NULL);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hImage);

    // use the source image's alpha channel for blending
    BLENDFUNCTION blend = { 0 };
    blend.BlendOp = AC_SRC_OVER;
    blend.SourceConstantAlpha = 255;
    blend.AlphaFormat = AC_SRC_ALPHA;

    // create a scaled bitmap
    HDC hdcScaled = CreateCompatibleDC(hdcScreen);
    HBITMAP hScaledBitmap = CreateCompatibleBitmap(hdcScreen, sizeSplash.cx, sizeSplash.cy);
    HBITMAP hOldScaledBitmap = (HBITMAP)SelectObject(hdcScaled, hScaledBitmap);

    SetStretchBltMode(hdcScaled, COLORONCOLOR);
    StretchBlt(hdcScaled, 0, 0, sizeSplash.cx, sizeSplash.cy, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    POINT ptZero = { 0 };
    // paint the window (in the right location) with the alpha-blended bitmap
    UpdateLayeredWindow(hwnd, hdcScreen, &ptOrigin, &sizeSplash,
        hdcScaled, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

    // delete temporary objects  
    SelectObject(hdcScaled, hOldScaledBitmap);
    DeleteObject(hScaledBitmap);
    DeleteDC(hdcScaled);
    SelectObject(hdcMem, hbmpOld);
    DeleteDC(hdcMem);
    ReleaseDC(NULL, hdcScreen);
}

void SelectOptions(HWND hwnd)
{
    std::wstring filePathWstr = SearchConfig();
    std::filesystem::path path(filePathWstr);
    if (!path.empty()) {
        if (path.extension() == L".cfg") {
            Animate* animateTmp = new Animate(filePathWstr);
			if (!animateTmp->GetLastError().empty()) {
				MessageBox(hwnd, animateTmp->GetLastError().c_str(), L"Error", MB_OK | MB_ICONERROR);
                animateTmp->~Animate();
				SelectOptions(hwnd); // Retry selection
            }
            else {
                animateO->~Animate(); // Clean up previous instance
                animateO = animateTmp;
            }
        }
        else if (path.extension() == L".png" || path.extension() == L".jpg") {
            Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(filePathWstr.c_str());
            HBITMAP hbm = nullptr;
            bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hbm);
            Animate* animateTmp = new Animate(hbm);
            delete bitmap;
            if (!animateTmp->GetLastError().empty()) {
                MessageBox(hwnd, animateTmp->GetLastError().c_str(), L"Error", MB_OK | MB_ICONERROR);
                animateTmp->~Animate();
                SelectOptions(hwnd); // Retry selection
            }
            else {
                animateO->~Animate(); // Clean up previous instance
                animateO = animateTmp;
            }
        }
        else {
            MessageBox(hwnd, L"Invalid file type selected. Please select a .cfg, .png, or .jpg file.", L"Error", MB_OK | MB_ICONERROR);
            SelectOptions(hwnd); // Retry selection
        }
    }
}

std::wstring SearchConfig()
{
    HRESULT hr;
    std::wstring filePath;

    // Initialize COM
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return filePath;

    // Create the File Open Dialog object
    IFileOpenDialog* pFileOpen = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL, IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

    if (SUCCEEDED(hr))
    {
        PWSTR documentsPath = nullptr;
        hr = SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &documentsPath);

        if (SUCCEEDED(hr))
        {
            // Convert std::wstring to IShellItem*
            IShellItem* pDefaultFolder = nullptr;
            hr = SHCreateItemFromParsingName(documentsPath, NULL, IID_PPV_ARGS(&pDefaultFolder));

            if (SUCCEEDED(hr))
            {
                // Set the default folder
                pFileOpen->SetDefaultFolder(pDefaultFolder);
                pDefaultFolder->Release();
            }

            CoTaskMemFree(documentsPath);
        }

        // Look for config file or image
        hr = pFileOpen->SetDefaultExtension(L"cfg;png;jpg");
        if (SUCCEEDED(hr))
        {
            // Show the Open dialog box
            hr = pFileOpen->Show(NULL);

            // Get the file name from the dialog box
            if (SUCCEEDED(hr))
            {
                IShellItem* pItem = nullptr;
                hr = pFileOpen->GetResult(&pItem);
                if (SUCCEEDED(hr))
                {
                    PWSTR pszFilePath = nullptr;
                    hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

                    // Convert PWSTR to std::wstring
                    if (SUCCEEDED(hr))
                    {
                        filePath = pszFilePath;
                        CoTaskMemFree(pszFilePath);
                    }
                    pItem->Release();
                }
            }
        }
        pFileOpen->Release();
    }

    // Uninitialize COM
    CoUninitialize();

    return filePath;
}
