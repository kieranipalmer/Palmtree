#pragma once
#include "Color.h"

class Texture
{
public:
	Texture(uint32_t Width, uint32_t Height);
	~Texture();
	Color* _pixelData;
	uint32_t GetWidth() { return _width; }
	uint32_t GetHeight() { return _height; }
private:
	uint32_t _height;
	uint32_t _width;
};