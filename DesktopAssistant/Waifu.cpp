#include "DesktopAssistant.h"
#include "Waifu.h"

Waifu::Waifu(const wchar_t* mainImagePath, const wchar_t* draggingImagePath)
	: state(0), image(NULL), draggingImage(NULL), posX(0), posY(0)
{
    Gdiplus::Bitmap mainImage(mainImagePath);
    mainImage.GetHBITMAP(NULL, &image);
    Gdiplus::Bitmap dragImage(draggingImagePath);
    dragImage.GetHBITMAP(NULL, &draggingImage);
    width = mainImage.GetWidth();
    height = mainImage.GetHeight();
    mainImage.GetHBITMAP(NULL, &image);
    if (image == NULL) {
        std::wcerr << L"Failed to load main image" << std::endl;
    }
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

HBITMAP Waifu::GetImage() const {
    switch (state)
    {
    case 1:
        // Dragging state
        return draggingImage;
        break;
    default:
        // Normal state
        return image;
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
    if (image) {
        DeleteObject(image);
    }
	if (draggingImage) {
		DeleteObject(draggingImage);
	}
}
