#include "DesktopAssistant.h"
#include "Waifu.h"

Waifu::Waifu(const std::wstring cfgPath)
	: state(0), posX(0), posY(0), width(0), height(0),
	idleCount(0), draggingCount(0), clickingCount(0),
    idleFilePattern(L"idle"), draggingFilePattern(L"dragging"), clickingFilePattern(L"click"),
    idleFps(2.5f), draggingFps(2.5f), clickingFps(2.5f),
    scaleMin(0.1f), scaleMax(10.0f), scaleStep(0.1f),
    idleImages(), draggingImages(), clickingImages()
{
	LoadConfig(cfgPath);
    
    HBITMAP hbm = idleImages[0];
    BITMAP bm;
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

int Waifu::GetStateCount() const {
    switch (state)
    {
    case 1:
        // Dragging state
        return draggingCount;
        break;
	case 2:
        // Clicking state
        return clickingCount;
        break;
    default:
        // Normal state
        return idleCount;
        break;
    }
}

float Waifu::GetStateFps() const {
    switch (state)
    {
    case 1:
        // Dragging state
        return draggingFps;
        break;
    case 2:
        // Clicking state
        return clickingFps;
        break;
    default:
        // Normal state
        return idleFps;
        break;
    }
}

HBITMAP Waifu::GetImage(int index) const {
    switch (state)
    {
    case 1:
        // Dragging state
        return draggingImages[index];
        break;
	case 2:
		// Clicking state
		return clickingImages[index];
		break;
    default:
        // Normal state
        return idleImages[index];
        break;
    }
}

Waifu::~Waifu() {
    for (HBITMAP bmp : idleImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : draggingImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : clickingImages)
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

void Waifu::LoadConfig(const std::wstring& configpath) {
    std::ifstream file(configpath);

    if (!file.is_open()) {
        std::wcerr << L"Could not open" << configpath << std::endl;
		exit(EXIT_FAILURE);
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
        idleFilePattern = config[L"idle"];
    }
    if (config.count(L"idle_count")) {
        idleCount = std::stoi(config[L"idle_count"]);
    }
    if (config.count(L"idle_fps")) {
        idleFps = std::stof(config[L"idle_fps"]);
    }
    if (config.count(L"dragging")) {
        draggingFilePattern = config[L"dragging"];
    }
    if (config.count(L"dragging_count")) {
        draggingCount = std::stoi(config[L"dragging_count"]);
    }
	if (config.count(L"dragging_fps")) {
		draggingFps = std::stof(config[L"dragging_fps"]);
	}
    if (config.count(L"click")) {
        clickingFilePattern = config[L"click"];
    }
    if (config.count(L"click_count")) {
        clickingCount = std::stoi(config[L"click_count"]);
    }
	if (config.count(L"clicking_fps")) {
		clickingFps = std::stof(config[L"clicking_fps"]);
	}
    if (config.count(L"scale_min")) {
        scaleMin = std::stof(config[L"scale_min"]);
    }
    if (config.count(L"scale_max")) {
        scaleMax = std::stof(config[L"scale_max"]);
    }
    if (config.count(L"scale_step")) {
        scaleStep = std::stof(config[L"scale_step"]);
    }

	// Load images based on the config
	idleImages = LoadVecBitmaps(idleCount, std::filesystem::path(configpath).parent_path().wstring(), idleFilePattern);
	draggingImages = LoadVecBitmaps(draggingCount, std::filesystem::path(configpath).parent_path().wstring(), draggingFilePattern);
	clickingImages = LoadVecBitmaps(clickingCount, std::filesystem::path(configpath).parent_path().wstring(), clickingFilePattern);
}
