#include <vector>
#include <string>
#include <sstream>

std::vector<HBITMAP> LoadVecBitmaps(int frameCount, const std::wstring& folder, const std::wstring& filePattern);

class Waifu {
private:
   int width;
   int height;
   int posX;
   int posY;
   int state; // 0: normal, 1: dragging
   std::vector<HBITMAP> vecBImages;
   std::vector<HBITMAP> vecBDraggingImages;
public:
   Waifu(const std::wstring folder, const std::wstring filePattern);
   ~Waifu();
   void SetPosition(int x, int y);
   void StartDragging();
   void StopDragging();
   bool IsDragging() const;
   int GetWidth() const;
   int GetHeight() const;
   int GetPosX() const;
   int GetPosY() const;
   HBITMAP GetImage(int index) const;
};
