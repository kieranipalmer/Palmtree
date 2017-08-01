#pragma once
#include "Texture.h"
#include <cstdint>
#include <cmath>
class PNGParser
{
public:
	static Texture* ParsePNG(const uint8_t* rawData);

private:

	struct PNGChunk
	{
		uint32_t Length;
		uint8_t ChunkType[4];
		const uint8_t* Data;
		uint32_t CRC;
	};

#pragma pack(push, 1)

	struct IHDRHeader {
		uint32_t Width;
		uint32_t Height;
		uint8_t BitDepth;
		uint8_t ColorType;
		uint8_t CompressionMethod;
		uint8_t FilterMethod;
		uint8_t InterlaceMethod;
	};

    struct ScanLine{
        uint8_t FilterByte;
        uint8_t* Data;
    };

#pragma pack(pop)

	static const int ZLIB_BUFFER_SIZE = 65536;
	static bool IsChunkTypeOf(PNGChunk& chunk, const char* type);
	static PNGChunk ReadChunk(const uint8_t* dataStart);
	static void ReadColorPalette(uint8_t* data,uint32_t dataLength,PNGChunk paletteChunk, IHDRHeader header, Texture* texture);
	static void ReadColorDataRGB(uint8_t* data,uint32_t dataLength, IHDRHeader header, Texture* texture, bool hasAlpha);
	static void ReadGreyData(uint8_t* data, uint32_t dataLength, IHDRHeader header, Texture* texture, bool hasAlpha);
	static uint32_t ReadData(const uint8_t* data, uint8_t bitDepth, bool isLittleEndian);
    static uint32_t GetCompressedPNGSize(const uint8_t* dataStart);
	static size_t GetPNGBufferSize(IHDRHeader header);
	static uint8_t GetBytesPerPixel(IHDRHeader header);
    static void RemoveFiltering(uint8_t* data,uint32_t dataLength,uint8_t bytesPerPixel,IHDRHeader header);
    static void ReverseUpFilter(uint8_t* data, uint32_t bytePos, uint32_t bytesPerPixel,uint32_t bytesPerScanline);
    static void ReverseSubFilter(uint8_t* data, uint32_t bytePos, uint32_t bytesPerPixel,uint32_t bytesPerScanline);
    static void ReverseAverageFilter(uint8_t* data,uint32_t bytePos, uint32_t bytesPerPixel, uint32_t bytesPerScanLine);
	static void ReversePaethFilter(uint8_t* data, uint32_t bytePos, uint32_t bytesPerPixel, uint32_t bytesPerScanLine);
	static uint32_t CalculatePeath(int32_t left, int32_t top, int32_t topLeft);

};

