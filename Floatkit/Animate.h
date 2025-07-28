#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <cwctype>
#include <algorithm>
#include <filesystem>
#include "Floatkit.h"

std::vector<HBITMAP> LoadVecBitmaps(int frameCount, const std::wstring& folder, const std::wstring& filePattern);
Gdiplus::Bitmap* LoadBitmapFromResource(UINT resourceID);

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
   int currentFrame;
   std::wstring lastError;
public:
	// Constructors
	Animate(); // Create an Animate object with resources from rc file
	Animate(const std::wstring cfgPath);
	Animate(HBITMAP hbm);
   ~Animate();

   void InitAnimate();

   // Logic
   void LoadConfig(const std::wstring& configPath);
   void SetPosition(int x, int y);
   void StopDragging();
   void StopClicking();
   void StartDragging();
   void StartClicking();
   bool IsDragging() const { return state == 1; }
   bool IsClicking() const { return state == 2; }
   HBITMAP GetImage(int index);
   int GetStateCount() const;
   float GetStateFps() const;
   void ClearLastError();
   void AddToLastError(const std::wstring& error);

   void SetCurrentFrame(int frame) { currentFrame = frame; }

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
   int GetCurrentFrame() const { return currentFrame; }
   std::wstring GetLastError() const { return lastError; }
};
