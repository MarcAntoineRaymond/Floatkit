#include "Animate.h"

void Animate::InitAnimate()
{
	state = State::Idle;
	posX = 0;
	posY = 0;
	width = 0;
	height = 0;
	idleCount = 1;
	draggingCount = 0; // No dragging frames by default
	clickingCount = 0; // No clicking frames by default
    movingCount = 0; // No moving frames by default
	idleFilePattern = L"idle";
	draggingFilePattern = L"dragging";
	clickingFilePattern = L"click";
    movingFilePattern = L"moving";
	idleFps = 2.5f;
	draggingFps = 2.5f;
	clickingFps = 2.5f;
    movingFps = 2.5f;
	currentFrame = 0;
	scaleMin = 0.1f;
	scaleMax = 10.0f;
	scaleStep = 0.1f;
	idleImages.clear();
	draggingImages.clear();
	clickingImages.clear();
    movingImages.clear();
    movingImagesFlipped.clear();
    movingDirection = Direction::Right;
    movingEnabled = false;
}

// Create an Animate object with resources from rc file
Animate::Animate() {
	InitAnimate();
	idleCount = 4;
	draggingCount = 10;
	clickingCount = 13;
    movingCount = 13;
    scaleMin = 5.0f;
	scaleStep = 5.0f;
	draggingFps = 5.0f;
    clickingFps = 10.0f;
    movingFps = 10.0f;
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
		delete bmp;
    }
    movingImages = clickingImages;
    for (HBITMAP bmp : movingImages) {
        movingImagesFlipped.push_back(FlipBitmapHorizontal(bmp));
    }
    movingEnabled = true;
}

Animate::Animate(const std::wstring cfgPath)
{
    InitAnimate();
    if (cfgPath.substr(cfgPath.find_last_of(L".") + 1) == L"gif") {
        idleImages = LoadGifFrames(cfgPath, idleFps);
        idleCount = idleImages.size();
    }
    else {
        LoadConfig(cfgPath);
        if (!GetLastError().empty()) {
            return;
        }
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
        state = State::Dragging;
}

void Animate::StopDragging() {
	if (state == State::Dragging)
		// If we were dragging, reset to idle state
		state = State::Idle;
}

void Animate::StartClicking() {
    if (!clickingImages.empty()) {
        state = State::Clicking;
		currentFrame = 0; // Reset frame to the first one when starting clicking
    }
}

void Animate::StopClicking() {
    if (state == State::Clicking)
        // If we were clicking, reset to idle state
        state = State::Idle;
}

void Animate::StartMoving(Direction direction) {
    if (!movingImages.empty()) {
        if (state != State::Moving) {
            currentFrame = 0; // Reset frame to the first one when starting to move
            state = State::Moving;
        }
        movingDirection = direction;
    }
}

void Animate::StopMoving() {
    if (state == State::Moving)
        // If we were moving, reset to idle state
        state = State::Idle;
}

int Animate::GetStateCount() const {
    switch (state)
    {
    case State::Dragging:
        // Dragging state
        return draggingCount;
        break;
	case State::Clicking:
        // Clicking state
        return clickingCount;
        break;
    case State::Moving:
        // Moving state
        return movingCount;
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
    case State::Dragging:
        // Dragging state
        return draggingFps;
        break;
    case State::Clicking:
        // Clicking state
        return clickingFps;
        break;
    case State::Moving:
        // Moving state
        return movingFps;
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
    case State::Dragging:
        // Dragging state
		if (index < 0 || index >= draggingImages.size())
			index = 0;
        return draggingImages[index];
        break;
	case State::Clicking:
		// Clicking state
		if (index < 0 || index >= clickingImages.size())
			index = 0;
        if (index == clickingImages.size() - 1)
			// If we are at the last frame of clicking, reset to idle state
			StopClicking();
		return clickingImages[index];
		break;
    case State::Moving:
        // Moving state
        if (index < 0 || index >= movingImages.size())
            index = 0;
        if (movingDirection == Direction::Left)
            return movingImagesFlipped[index];
        return movingImages[index];
        break;
    default:
        // Normal state
		if (index < 0 || index >= idleImages.size())
			index = 0;
        return idleImages[index];
        break;
    }
}

HBITMAP FlipBitmapHorizontal(HBITMAP hOriginal)
{
    if (!hOriginal)
        return nullptr;

    BITMAP bm{};
    if (GetObject(hOriginal, sizeof(BITMAP), &bm) == 0)
        return nullptr;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcSrc = CreateCompatibleDC(hdcScreen);
    HDC hdcDst = CreateCompatibleDC(hdcScreen);

    if (!hdcSrc || !hdcDst)
    {
        if (hdcSrc) DeleteDC(hdcSrc);
        if (hdcDst) DeleteDC(hdcDst);
        ReleaseDC(nullptr, hdcScreen);
        return nullptr;
    }

    HBITMAP hFlipped = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);
    if (!hFlipped)
    {
        DeleteDC(hdcSrc);
        DeleteDC(hdcDst);
        ReleaseDC(nullptr, hdcScreen);
        return nullptr;
    }

    HGDIOBJ oldSrc = SelectObject(hdcSrc, hOriginal);
    HGDIOBJ oldDst = SelectObject(hdcDst, hFlipped);

    SetStretchBltMode(hdcDst, COLORONCOLOR);

    // Copy original into destination, mirrored horizontally
    BOOL ok = StretchBlt(
        hdcDst,
        0, 0,
        bm.bmWidth, bm.bmHeight,
        hdcSrc,
        bm.bmWidth - 1, 0,
        -bm.bmWidth, bm.bmHeight,
        SRCCOPY
    );

    SelectObject(hdcSrc, oldSrc);
    SelectObject(hdcDst, oldDst);

    DeleteDC(hdcSrc);
    DeleteDC(hdcDst);
    ReleaseDC(nullptr, hdcScreen);

    if (!ok)
    {
        DeleteObject(hFlipped);
        return nullptr;
    }

    return hFlipped;
}

Animate::~Animate() {
    for (HBITMAP bmp : idleImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : draggingImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : clickingImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : movingImages)
        if (bmp) DeleteObject(bmp);
    for (HBITMAP bmp : movingImagesFlipped)
        if (bmp) DeleteObject(bmp);
}

std::vector<HBITMAP> LoadVecBitmaps(int frameCount, const std::filesystem::path& filePath)
{
    std::vector<HBITMAP> bitmaps;
    for (int i = 0; i < frameCount; ++i)
    {
        std::wstringstream path;
        path << filePath << "_" << i << L".png";

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
    if (config.count(L"moving")) {
        movingFilePattern = config[L"moving"];
    }
    if (config.count(L"moving_count")) {
        movingCount = std::stoi(config[L"moving_count"]);
    }
    if (config.count(L"moving_fps")) {
        movingFps = std::stof(config[L"moving_fps"]);
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

    std::filesystem::path base = std::filesystem::path(configpath).parent_path();

    // --- IDLE ---
    std::filesystem::path idlePath = base / idleFilePattern;
    std::filesystem::path idleGif = idlePath;
    idleGif += L".gif";

    if (std::filesystem::exists(idleGif)) {
        idleImages = LoadGifFrames(idleGif.wstring(), idleFps);
        idleCount = idleImages.size();
    }
    else {
        idleImages = LoadVecBitmaps(idleCount, idlePath.wstring());
    }

    // --- DRAGGING ---
    std::filesystem::path draggingPath = base / draggingFilePattern;
    std::filesystem::path draggingGif = draggingPath;
    draggingGif += L".gif";

    if (std::filesystem::exists(draggingGif)) {
        draggingImages = LoadGifFrames(draggingGif.wstring(), draggingFps);
        draggingCount = draggingImages.size();
    }
    else {
        draggingImages = LoadVecBitmaps(draggingCount, draggingPath.wstring());
    }

    // --- CLICKING ---
    std::filesystem::path clickingPath = base / clickingFilePattern;
    std::filesystem::path clickingGif = clickingPath;
    clickingGif += L".gif";

    if (std::filesystem::exists(clickingGif)) {
        clickingImages = LoadGifFrames(clickingGif.wstring(), clickingFps);
        clickingCount = clickingImages.size();
    }
    else {
        clickingImages = LoadVecBitmaps(clickingCount, clickingPath.wstring());
    }

    // --- MOVING ---
    std::filesystem::path movingPath = base / movingFilePattern;
    std::filesystem::path movingGif = movingPath;
    movingGif += L".gif";

    if (std::filesystem::exists(movingGif)) {
        movingImages = LoadGifFrames(movingGif.wstring(), movingFps);
        movingCount = movingImages.size();
    }
    else {
        movingImages = LoadVecBitmaps(movingCount, movingPath.wstring());
    }

    for (HBITMAP bmp : movingImages) {
        movingImagesFlipped.push_back(FlipBitmapHorizontal(bmp));
    }

    if (idleCount < 1) {
        std::wstring error = L"Invalid idle_count in config: " + std::to_wstring(idleCount) + L"\nidle_count cannot be inferior to 1";
        AddToLastError(error);
        return;
    }

    // Check if moving enabled after loading images for default
    if (config.count(L"moving_enabled")) {
        if (config[L"moving_enabled"] == L"true" || config[L"moving_enabled"] == L"True")
            movingEnabled = true;
    }
    else {
        if (!movingImages.empty())
            movingEnabled = true;
    }
}

std::vector<HBITMAP> LoadGifFrames(const std::filesystem::path filePath, float& outFps)
{
    std::vector<HBITMAP> frames;

    Gdiplus::Bitmap* gif = Gdiplus::Bitmap::FromFile(filePath.c_str());
    if (!gif || gif->GetLastStatus() != Gdiplus::Ok) {
        delete gif;
        return frames;
    }

    GUID dimensionID;
    UINT count = gif->GetFrameDimensionsCount();
    if (count == 0) {
        delete gif;
        return frames;
    }

    gif->GetFrameDimensionsList(&dimensionID, 1);

    UINT frameCount = gif->GetFrameCount(&dimensionID);

    // Get frame delays (PropertyTagFrameDelay = 0x5100)
    UINT size = gif->GetPropertyItemSize(PropertyTagFrameDelay);
    std::vector<BYTE> buffer(size);

    Gdiplus::PropertyItem* propItem = reinterpret_cast<Gdiplus::PropertyItem*>(buffer.data());
    gif->GetPropertyItem(PropertyTagFrameDelay, size, propItem);

    UINT* delays = reinterpret_cast<UINT*>(propItem->value);

    // Extract frames
    for (UINT i = 0; i < frameCount; ++i)
    {
        gif->SelectActiveFrame(&dimensionID, i);

        HBITMAP hbm = nullptr;
        gif->GetHBITMAP(Gdiplus::Color(0, 0, 0), &hbm);
        frames.push_back(hbm);
    }

    // Compute FPS (GIF delay is in 1/100 sec)
    if (frameCount > 0) {
        float avgDelay = 0.0f;
        for (UINT i = 0; i < frameCount; ++i)
            avgDelay += delays[i];

        avgDelay /= frameCount;

        if (avgDelay > 0)
            outFps = 100.0f / avgDelay;
    }

    delete gif;
    return frames;
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
