#pragma once
#include "Texture.h"
#include <cstdint>
class BitmapParser
{
public:
	static Texture* ParseBitmap(unsigned char* rawData);
private:
#pragma pack(push, 1)
	struct BitmapHeader {
		uint16_t HeaderField;
		uint32_t Size;
		uint32_t Padding;
		uint32_t DataOffset;
	};


	struct BitmapInfoHeader {
		uint32_t HeaderSize;
		int32_t Width;
		int32_t Height;
		uint16_t ColorPlanes;
		uint16_t BitsPerPixel;
		uint32_t CompressionMethod;
		uint32_t ImageSize;
		int32_t HorizontalPPM;
		int32_t VerticalPPM;
		uint32_t ColorsInPalette;
		uint32_t ImportantColors;
	};

	struct BitmapInfoV4Header {
		BitmapInfoHeader InfoHeader;
		uint32_t RedMask;
		uint32_t GreenMask;
		uint32_t BlueMask;
		uint32_t AlphaMask;
		uint32_t ColorSpaceType;
	};
#pragma pack(pop)
};

