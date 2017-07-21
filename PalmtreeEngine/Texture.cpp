#include "Texture.h"
#include <cstdlib>


Texture::Texture(uint32_t Width, uint32_t Height)
{
	_pixelData = (Color*)std::malloc(Width*Height*sizeof(Color));
	_width = Width;
	_height = Height;
}

Texture::~Texture()
{
}
