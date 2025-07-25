#include "DesktopAssistant.h"
#include "Waifu.h"

Waifu::Waifu(const std::wstring folder, const std::wstring filePattern)
	: state(0), posX(0), posY(0), vecBImages(), vecBDraggingImages()
{
    vecBImages = LoadVecBitmaps(2, folder, filePattern);
	vecBDraggingImages = LoadVecBitmaps(2, folder, filePattern + L"_lean");
    
    HBITMAP hbm = vecBImages[0];
    BITMAP bm;
    CHAR szBuffer[255];
    GetObject(hbm, sizeof(BITMAP), &bm);
    width = bm.bmWidth;
    height = bm.bmHeight;

    POINT ptZero = { 0 };
    HMONITOR hmonPrimary = MonitorFromPoint(ptZero, MONITOR_DEFAULTTOPRIMARY);
    MONITORINFO monitorinfo = { 0 };
    monitorinfo.cbSize = sizeof(monitorinfo);
    GetMonitorInfo(hmonPrimary, &monitorinfo);

    // Create the splash screen in the bottom right corner of the primary work area  
    const RECT& rcWork = monitorinfo.rcWork;
    posX = rcWork.right - width;
    posY = rcWork.bottom - width;
}

void Waifu::SetPosition(int x, int y) {
	posX = x;
	posY = y;
}

bool Waifu::IsDragging() const {
	return state == 1;
}

void Waifu::StartDragging() {
	state = 1; // Set state to dragging
}

void Waifu::StopDragging() {
	state = 0; // Set state to normal
}

HBITMAP Waifu::GetImage(int index) const {
    switch (state)
    {
    case 1:
        // Dragging state
        return vecBDraggingImages[index];
        break;
    default:
        // Normal state
        return vecBImages[index];
        break;
    }
}

int Waifu::GetWidth() const {
	return width;
}

int Waifu::GetHeight() const {
	return height;
}

int Waifu::GetPosX() const {
    return posX;
}

int Waifu::GetPosY() const {
    return posY;
}

Waifu::~Waifu() {
    for (HBITMAP bmp : vecBImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : vecBDraggingImages)
        if (bmp) DeleteObject(bmp);
}

std::vector<HBITMAP> LoadVecBitmaps(int frameCount, const std::wstring& folder, const std::wstring& filePattern)
{
    std::vector<HBITMAP> bitmaps;
    for (int i = 0; i < frameCount; ++i)
    {
        std::wstringstream path;
        path << folder << L"\\" << filePattern << "_" << i << L".png";

        Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromFile(path.str().c_str());
        if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
            delete bitmap;
            bitmaps.push_back(nullptr);
            continue;
        }

        HBITMAP hbm = nullptr;
        bitmap->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hbm);
        bitmaps.push_back(hbm);
        delete bitmap;
    }

    return bitmaps;
}
