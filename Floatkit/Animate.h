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
HBITMAP FlipBitmapHorizontal(HBITMAP hOriginal);

enum class Direction
{
	Left,
	Right
};

enum class State
{
	Idle,
	Dragging,
	Clicking,
	Moving
};

class Animate {
private:
   int width;
   int height;
   int posX;
   int posY;
   State state;
   std::vector<HBITMAP> idleImages;
   int idleCount;
   std::wstring idleFilePattern;
   float idleFps;
   std::vector<HBITMAP> draggingImages;
   int draggingCount;
   std::wstring draggingFilePattern;
   float draggingFps;
   std::vector<HBITMAP> clickingImages;
   int clickingCount;
   std::wstring clickingFilePattern;
   float clickingFps;
   std::vector<HBITMAP> movingImages;
   int movingCount;
   std::wstring movingFilePattern;
   float movingFps;
   Direction movingDirection;
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
   void StartMoving(Direction direction);
   void StopMoving();
   bool IsIdle() const { return state == State::Idle; }
   bool IsDragging() const { return state == State::Dragging; }
   bool IsClicking() const { return state == State::Clicking; }
   bool IsMoving() const { return state == State::Moving; }
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
