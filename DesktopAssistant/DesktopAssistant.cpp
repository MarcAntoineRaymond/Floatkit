#include "DesktopAssistant.h"
#include "Waifu.h"

Waifu* waifu = nullptr;

ULONG_PTR gdiplusToken;
HMENU hPopMenu;
BOOL bStopped = FALSE;
HINSTANCE hInst;
int nCmd;

POINT dragOffset = { 0, 0 };
float g_scale = 1.0f;
int currentFrame = 0;

ULONGLONG lastFrameTime = GetTickCount64();


//config
std::wstring idle = L"roxy";
int idle_count = 2;
std::wstring dragging = L"dragging";
int dragging_count = 0;
std::wstring click = L"click";
int click_count = 0;
float fps = 60.0f; // 60 FPS ~ 1/60 * 1000 ms per frame
float animation_fps = 2.5f; // 2.5 FPS ~ 1/2.5 * 1000 ms per frame
float scale_min = 0.1f;
float scale_max = 10.0f;
float scale_step = 0.1f;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    waifu = new Waifu(L"assets", idle);

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
        (*waifu).StartDragging();
        InvalidateCursor();
        clickOffset.x = LOWORD(lParam);
        clickOffset.y = HIWORD(lParam);
        return 0;

    case WM_MOUSEMOVE:
        if ((*waifu).IsDragging()) {
            POINT pt;
            GetCursorPos(&pt);
			int pos_x = pt.x - clickOffset.x;
			int pos_y = pt.y - clickOffset.y;
			(*waifu).SetPosition(pos_x, pos_y);
            SetWindowPos(hwnd, HWND_TOPMOST, pos_x, pos_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        return 0;
    case WM_MOUSEWHEEL:
        if ((*waifu).IsDragging())
        {
            short delta = GET_WHEEL_DELTA_WPARAM(wParam);
            float newScale = g_scale + (delta > 0 ? scale_step : -scale_step);
            if (newScale < scale_min) newScale = scale_min;
            if (newScale > scale_max) newScale = scale_max;
            if (newScale != g_scale) {
                g_scale = newScale;
            }
        }
        return 0;
    case WM_LBUTTONUP:
        ReleaseCapture();
        (*waifu).StopDragging();
        InvalidateCursor();
        return 0;
    case WM_USER_SHELLICON:
        // systray msg callback 
        switch (LOWORD(lParam))
        {
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
            SelectOptions();
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
            if ((*waifu).IsDragging())
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
    if (now - lastFrameTime >= 1000/animation_fps) {
        currentFrame = (currentFrame + 1) % idle_count;
        lastFrameTime = now;
    }

    HBITMAP hImage = (*waifu).GetImage(currentFrame);
    if (!hImage) return;
    BITMAP bm;
    GetObject(hImage, sizeof(bm), &bm);
    SIZE sizeSplash = {
        static_cast<LONG>(bm.bmWidth * g_scale),
        static_cast<LONG>(bm.bmHeight * g_scale)
    };

    POINT ptOrigin;
    ptOrigin.x = (*waifu).GetPosX();
    ptOrigin.y = (*waifu).GetPosY();

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

void SelectOptions()
{
    std::wstring cfgPathWstr = SearchConfig();
	LoadConfig(cfgPathWstr);
    waifu = new Waifu(std::filesystem::path(cfgPathWstr).parent_path().wstring(), idle);
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

        // Look for config file
        hr = pFileOpen->SetDefaultExtension(L"cfg");
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

// Trim whitespace from wstring
static inline void trim(std::wstring& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](wchar_t ch) {
        return !std::iswspace(ch);
        }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](wchar_t ch) {
        return !std::iswspace(ch);
        }).base(), s.end());
}

// Convert UTF-8 to std::wstring using Windows API
std::wstring utf8_to_wstring(const std::string& utf8) {
    if (utf8.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, nullptr, 0);
    if (size_needed == 0) return L"";

    std::wstring result(size_needed - 1, 0); // -1 to exclude null terminator
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, &result[0], size_needed);
    return result;
}

void LoadConfig(std::wstring configpath) {
    std::ifstream file(configpath);

    if (!file.is_open()) {
        std::wcerr << L"Could not open config.txt" << std::endl;
    }

    std::unordered_map<std::wstring, std::wstring> config;
    std::string line;

    while (std::getline(file, line)) {
        auto wline = utf8_to_wstring(line);

        size_t pos = wline.find(L'=');
        if (pos == std::wstring::npos) continue;

        std::wstring key = wline.substr(0, pos);
        std::wstring value = wline.substr(pos + 1);

        trim(key);
        trim(value);

        if (!key.empty()) {
            config[key] = value;
        }
    }

    // Set variables from config map
    if (config.count(L"idle")) {
        idle = config[L"idle"];
    }
    if (config.count(L"idle_count")) {
        idle_count = std::stoi(config[L"idle_count"]);
    }
    if (config.count(L"dragging")) {
        dragging = config[L"dragging"];
    }
    if (config.count(L"dragging_count")) {
        dragging_count = std::stoi(config[L"dragging_count"]);
    }
    if (config.count(L"click")) {
        click = config[L"click"];
    }
    if (config.count(L"click_count")) {
        click_count = std::stoi(config[L"click_count"]);
    }
    if (config.count(L"animation_fps")) {
        animation_fps = std::stof(config[L"animation_fps"]);
    }
    if (config.count(L"scale_min")) {
        scale_min = std::stof(config[L"scale_min"]);
    }
    if (config.count(L"scale_max")) {
        scale_max = std::stof(config[L"scale_max"]);
    }
    if (config.count(L"scale_step")) {
        scale_step = std::stof(config[L"scale_step"]);
    }
}
