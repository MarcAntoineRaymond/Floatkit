#include "Animate.h"

void Animate::InitAnimate()
{
	state = 0;
	posX = 0;
	posY = 0;
	width = 0;
	height = 0;
	idleCount = 1;
	draggingCount = 0; // No dragging frames by default
	clickingCount = 0; // No clicking frames by default
	idleFilePattern = L"idle";
	draggingFilePattern = L"dragging";
	clickingFilePattern = L"click";
	idleFps = 2.5f;
	draggingFps = 2.5f;
	clickingFps = 2.5f;
	currentFrame = 0;
	scaleMin = 0.1f;
	scaleMax = 10.0f;
	scaleStep = 0.1f;
	idleImages.clear();
	draggingImages.clear();
	clickingImages.clear();
}

// Create an Animate object with resources from rc file
Animate::Animate() {
	InitAnimate();
	idleCount = 4;
	draggingCount = 10;
	clickingCount = 13;
    scaleMin = 5.0f;
	scaleStep = 5.0f;
	draggingFps = 5.0f;
    clickingFps = 10.0f;
	// Load default images from resources
    for (int i = 0; i < idleCount + draggingCount + clickingCount; i++) {
        Gdiplus::Bitmap* bmp = LoadBitmapFromResource(IDB_PNG1 + i);
        HBITMAP hbm = nullptr;
        bmp->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hbm);
		if (i < idleCount)
			idleImages.push_back(hbm);
		else if (i < idleCount + draggingCount)
			draggingImages.push_back(hbm);
		else
			clickingImages.push_back(hbm);
		idleImages.push_back(hbm);
		delete bmp;
    }
}

Animate::Animate(const std::wstring cfgPath)
{
    InitAnimate();
    LoadConfig(cfgPath);
	if (!GetLastError().empty()) {
		return;
	}
    
    HBITMAP hbm = idleImages[0];
    BITMAP bm;
    GetObject(hbm, sizeof(BITMAP), &bm);
    width = bm.bmWidth;
    height = bm.bmHeight;
	if (draggingImages.empty()) {
		draggingImages.push_back(hbm);
		draggingCount = 1;
	}
}

Animate::Animate(HBITMAP hbm)
{
	InitAnimate();
    idleImages = { hbm };
    BITMAP bm;
    GetObject(hbm, sizeof(BITMAP), &bm);
    width = bm.bmWidth;
    height = bm.bmHeight;
    if (draggingImages.empty()) {
        draggingImages.push_back(hbm);
		draggingCount = 1;
    }
}

void Animate::SetPosition(int x, int y) {
	posX = x;
	posY = y;
}

void Animate::StartDragging() {
    if (!draggingImages.empty())
        state = 1;
}

void Animate::StopDragging() {
	if (state == 1)
		// If we were dragging, reset to idle state
		state = 0;
}

void Animate::StartClicking() {
    if (!clickingImages.empty()) {
        state = 2;
		currentFrame = 0; // Reset frame to the first one when starting clicking
    }
}

void Animate::StopClicking() {
    if (state == 2)
        // If we were clicking, reset to idle state
        state = 0;
}

int Animate::GetStateCount() const {
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

float Animate::GetStateFps() const {
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

HBITMAP Animate::GetImage(int index) {
    switch (state)
    {
    case 1:
        // Dragging state
		if (index < 0 || index >= draggingImages.size())
			index = 0;
        return draggingImages[index];
        break;
	case 2:
		// Clicking state
		if (index < 0 || index >= clickingImages.size())
			index = 0;
        if (index == clickingImages.size() - 1)
			// If we are at the last frame of clicking, reset to idle state
			StopClicking();
		return clickingImages[index];
		break;
    default:
        // Normal state
		if (index < 0 || index >= idleImages.size())
			index = 0;
        return idleImages[index];
        break;
    }
}

Animate::~Animate() {
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

void Animate::LoadConfig(const std::wstring& configpath) {
    std::ifstream file(configpath);

    if (!file.is_open()) {
		AddToLastError(L"Could not open config file: " + configpath);
		return;
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
	if (config.count(L"click_fps")) {
		clickingFps = std::stof(config[L"click_fps"]);
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

	if (idleCount < 1) {
		std::wstring error = L"Invalid idle_count in config: " + std::to_wstring(idleCount) + L"\nidle_count cannot be inferior to 1";
		AddToLastError(error);
        return;
	}

	// Load images based on the config
	idleImages = LoadVecBitmaps(idleCount, std::filesystem::path(configpath).parent_path().wstring(), idleFilePattern);
	draggingImages = LoadVecBitmaps(draggingCount, std::filesystem::path(configpath).parent_path().wstring(), draggingFilePattern);
	clickingImages = LoadVecBitmaps(clickingCount, std::filesystem::path(configpath).parent_path().wstring(), clickingFilePattern);
}

// Return latest error for the animate object and clear the errors
void Animate::ClearLastError()
{
    lastError.clear();
}

void Animate::AddToLastError(const std::wstring& error)
{
	if (!lastError.empty()) {
		lastError += L"\n";
	}
	lastError += error;
}
