class Waifu {  
private:  
   int width;  
   int height;  
   int posX;  
   int posY;   
   HBITMAP draggingImage;
   HBITMAP image;
   int state; // 0: normal, 1: dragging  

public:  
   Waifu(const wchar_t* mainImagePath, const wchar_t* draggingImagePath);
   ~Waifu();  
   void SetPosition(int x, int y);  
   void StartDragging();  
   void StopDragging();  
   bool IsDragging() const;  
   int GetWidth() const;  
   int GetHeight() const;
   int GetPosX() const;
   int GetPosY() const;
   HBITMAP GetImage() const;  
};