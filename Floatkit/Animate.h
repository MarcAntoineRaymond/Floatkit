#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <cwctype>
#include <algorithm>
#include <filesystem>

std::vector<HBITMAP> LoadVecBitmaps(int frameCount, const std::wstring& folder, const std::wstring& filePattern);

class Animate {
private:
   int width;
   int height;
   int posX;
   int posY;
   int state; // 0: normal, 1: dragging, 2: clicking
   std::vector<HBITMAP> idleImages;
   int idleCount;
   std::wstring idleFilePattern;
   std::vector<HBITMAP> draggingImages;
   int draggingCount;
   std::wstring draggingFilePattern;
   std::vector<HBITMAP> clickingImages;
   int clickingCount;
   std::wstring clickingFilePattern;
   float idleFps;
   float draggingFps;
   float clickingFps;
   float scaleMin;
   float scaleMax;
   float scaleStep;
public:
	Animate(const std::wstring cfgPath);
   ~Animate();

   // Logic
   void LoadConfig(const std::wstring& configPath);
   void SetPosition(int x, int y);
   void StartDragging();
   void StopDragging();
   bool IsDragging() const;
   HBITMAP GetImage(int index) const;
   int GetStateCount() const;
   float GetStateFps() const;

   // Getters
   int GetWidth() const { return width; }
   int GetHeight() const { return height; }
   int GetPosX() const { return posX; }
   int GetPosY() const { return posY; }
   float GetIdleFps() const { return idleFps; }
   float GetDraggingFps() const { return draggingFps; }
   float GetClickingFps() const { return clickingFps; }
   float GetScaleMin() const { return scaleMin; }
   float GetScaleMax() const { return scaleMax; }
   float GetScaleStep() const { return scaleStep; }
};
