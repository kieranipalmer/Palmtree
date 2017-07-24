#include "PNGParser.h"
#include "EndianessUtilityFunctions.h"
#include <cstring>
#include <cstdlib>
#include <zlib.h>

Texture * PNGParser::ParsePNG(const uint8_t * rawData) {
    //Check platform endianess, PNG uses big endian so we need to know if we should reverse data
    bool isLittleEndian = EdianessUtility::IsLittleEndian();

    //Initalise ZLib
    int ret;
    uint32_t have = 0;
    z_stream stream;
    uint8_t outputBuffer[ZLIB_BUFFER_SIZE];
    uint32_t bytesRead=0;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.avail_in = 0;
    stream.next_in = Z_NULL;
    ret = inflateInit(&stream);
    if (ret != Z_OK) {
        //Initalisation of ZLib failed
        //TODO: LOG ERROR
        return nullptr;
    }

    //First eight bytes of PNG are the magic number
    uint32_t currentIndex = 8;
    PNGChunk currentChunk = ReadChunk(rawData + currentIndex);
    if (!IsChunkTypeOf(currentChunk, "IHDR")) {
        //Invalid PNG emit error
        return nullptr;
    }

    IHDRHeader ihdrHeader;
    std::memcpy(&ihdrHeader, currentChunk.Data, 13);
    if (isLittleEndian) {
        EdianessUtility::SwapEndian(ihdrHeader.Width);
        EdianessUtility::SwapEndian(ihdrHeader.Height);
    }


    //Create buffer for storing compressed ZLIB stream
    uint32_t compressedDataSize = GetCompressedPNGSize(rawData+8);
    uint8_t* compressedData = (uint8_t*)std::malloc(compressedDataSize);
    uint32_t compressedDataOffset = 0;

	//Create buffer for storing decompressed png data
	uint8_t* pngData = (uint8_t*)std::malloc(GetPNGBufferSize(ihdrHeader));
    std::memset(pngData,0,GetPNGBufferSize(ihdrHeader));
    Texture *retVal = new Texture(ihdrHeader.Width, ihdrHeader.Height);
    PNGChunk paletteChunk;

    while (!IsChunkTypeOf(currentChunk, "IEND")) {

        //Process new chunk
        if (IsChunkTypeOf(currentChunk, "PLTE")) {
            paletteChunk = currentChunk;
        } else if (IsChunkTypeOf(currentChunk, "IDAT")) {
            std::memcpy(compressedData+compressedDataOffset,currentChunk.Data,currentChunk.Length);
            compressedDataOffset += currentChunk.Length;

        }

        //Move to the start of the next chunk, +12 as chunk length doesn't account
        //for chunk layout data
        currentIndex += currentChunk.Length + 12;
        currentChunk = ReadChunk(rawData + currentIndex);

    }


    //Decompress data
    stream.avail_in = compressedDataSize;
    stream.next_in = compressedData;

    do {
        stream.avail_out = ZLIB_BUFFER_SIZE;
        stream.next_out = outputBuffer;
        ret = inflate(&stream, Z_NO_FLUSH);
        have = ZLIB_BUFFER_SIZE - stream.avail_out;
        std::memcpy(pngData+bytesRead,outputBuffer,have);
        bytesRead += have;
    } while (ret != Z_STREAM_END);
    inflateEnd(&stream);

    //Defilter data
    uint8_t bpp = 0;
    switch(ihdrHeader.ColorType)
    {
        case 0:
            bpp = ihdrHeader.BitDepth/8;
            break;
        case 2:
            bpp = ihdrHeader.BitDepth/8*3;
            break;
        case 3:
            bpp = ihdrHeader.BitDepth/8;
            break;
        case 4:
            bpp = ihdrHeader.BitDepth/8*2;
            break;
        case 6:
            bpp = ihdrHeader.BitDepth/8*4;
            break;
    }
    RemoveFiltering(pngData,bytesRead,bpp,ihdrHeader);

    //Parse texture data
    switch(ihdrHeader.ColorType)
    {
        //Grey
        case 0:
            break;
        //RGB
        case 2:
            ReadColorDataRGB(pngData,bytesRead,ihdrHeader,retVal,false);
            break;
        //Pallete
        case 3:
            ReadColorPalette(pngData,bytesRead,paletteChunk,ihdrHeader,retVal);
            break;
        //Greyscale with Alpha
        case 4:

            break;
        //RGBA
        case 6:
            ReadColorDataRGB(pngData,bytesRead,ihdrHeader,retVal,true);
            break;

    }

    std::free(compressedData);
    std::free(pngData);
	return retVal;
}



bool PNGParser::IsChunkTypeOf(PNGChunk & chunk, const char * type)
{
	return strncmp((char*)chunk.ChunkType, type,4)==0;
}

PNGParser::PNGChunk PNGParser::ReadChunk(const uint8_t * dataStart)
{
	bool isLittleEndian = EdianessUtility::IsLittleEndian();

	PNGChunk chunk;
	//Length is the first 4 bytes
	std::memcpy(&chunk.Length, dataStart, 4);
	if (isLittleEndian)
	{
		EdianessUtility::SwapEndian(chunk.Length);
	}
        
	//Chunk type is the next 4 bytes
	std::memcpy(&chunk.ChunkType, dataStart + 4, 4);
	//The data starts after the Chunk Type, not copying as we can just process in place by 
	//retaining a pointer
	chunk.Data = dataStart + 8;
	//CRC is the 4 bytes afer the data field
	std::memcpy(&chunk.CRC, dataStart+chunk.Length+4, 4);

	if (isLittleEndian)
	{
		EdianessUtility::SwapEndian(chunk.CRC);
	}
	return chunk;
}

void PNGParser::ReadColorPalette(uint8_t* data,uint32_t dataLength, PNGChunk paletteChunk, IHDRHeader header, Texture * texture)
{
	bool isLittleEndian = EdianessUtility::IsLittleEndian();
    uint32_t bytesPerScanline = (header.Width)*header.BitDepth/8 + 1;

    for (size_t i = 0; i < dataLength; i++)
    {
        //Skip filter bytes
        if(i%bytesPerScanline==0)
        {
            i++;
        }
		uint32_t index = ReadData(data + i, header.BitDepth, isLittleEndian)*3;
		Color col((paletteChunk.Data[index]) / 255.f, (paletteChunk.Data[index+1]) / 255.f, (paletteChunk.Data[index+2]) / 255.f,0.f);
        texture->_pixelData[i-(i/bytesPerScanline)]=col;
	}
}

void PNGParser::ReadColorDataRGB(uint8_t* data, uint32_t dataLength, IHDRHeader header, Texture* texture, bool hasAlpha)
{
	bool isLittleEndian = EdianessUtility::IsLittleEndian();
	int nBytesPerSample = header.BitDepth / 8;
    //Bytes per pixel, if we have alpha 4 samples otherwise 3
    uint8_t bpp = nBytesPerSample * hasAlpha?4:3;

    //+1 for filter byte
    uint32_t bytesPerScanline = (header.Width)*bpp + 1;
    uint32_t currentPixel = 0;
    float sampleMax = (float)(2<<(header.BitDepth-1))-1;
	//Loop over every color entry
	for (size_t i = 0; i < dataLength; i+=bpp)
	{
		//Skip the first byte of each scanline as that is filter byte
		if (i % bytesPerScanline == 0)
		{
			i++;
		}

		Color newColor(ReadData(data + i, header.BitDepth, isLittleEndian)/sampleMax,
			ReadData(data + i + nBytesPerSample, header.BitDepth, isLittleEndian)/sampleMax,
			ReadData(data + i + nBytesPerSample * 2, header.BitDepth, isLittleEndian)/sampleMax,
			hasAlpha?ReadData(data + i + nBytesPerSample * 3, header.BitDepth, isLittleEndian)/sampleMax:1.f);
        texture->_pixelData[currentPixel] = newColor;
        currentPixel++;
	}
}

uint32_t PNGParser::ReadData(const uint8_t * data, uint8_t bitDepth, bool isLittleEndian)
{
	switch (bitDepth)
	{
        case 2:
        case 4:
		case 8:
			return data[0];
		case 16:
			if (isLittleEndian)
			{
				return data[0] << 8 | data[1];
			}
			else
			{
				return data[1] << 8 | data[0];
			}


		default:
		break;
	}
	return 0;
}

size_t PNGParser::GetPNGBufferSize(IHDRHeader header)
{
	return (header.Width + 1)*header.Height;
}

uint32_t PNGParser::GetCompressedPNGSize(const uint8_t* dataStart) {
    uint32_t retVal = 0;
    uint32_t currentIndex=0;
    PNGChunk currentChunk = ReadChunk(dataStart);
    while(!IsChunkTypeOf(currentChunk,"IEND"))
    {
        if(IsChunkTypeOf(currentChunk,"IDAT"))
        {
            retVal += currentChunk.Length;
        }
        //Move to the start of the next chunk, +12 as chunk length doesn't account
        //for chunk layout data
        currentIndex += currentChunk.Length + 12;
        currentChunk = ReadChunk(dataStart + currentIndex);
    }

    return retVal;
}

void PNGParser::RemoveFiltering(uint8_t *data, uint32_t dataLength,uint8_t bytesPerPixel, IHDRHeader header) {
    bool isLittleEndian = EdianessUtility::IsLittleEndian();
    int nBytesPerSample = header.BitDepth / 8;
    //+1 for filter byte
    uint32_t bytesPerScanline = (header.Width)*bytesPerPixel + 1;
    uint8_t currentFilterMode = 0;

    for(size_t i=0;i<dataLength;i++)
    {
        //Read current filter byte
        if(i%bytesPerScanline==0)
        {
            currentFilterMode = data[i];
            i++;
        }

        switch(currentFilterMode)
        {
            //Sub filter
            case 1:
                ReverseSubFilter(data,i,bytesPerPixel,bytesPerScanline);
                break;
            //Up filter
            case 2:
                ReverseUpFilter(data,i,bytesPerPixel,bytesPerScanline);
                break;
            //Average
            case 3:
                ReverseAverageFilter(data,i,bytesPerPixel,bytesPerScanline);
                break;
            //Paeth
            case 4:
                ReversePaethFilter(data,i,bytesPerPixel,bytesPerScanline);
                break;
        }

    }
}

void PNGParser::ReverseSubFilter(uint8_t * data, uint32_t bytePos, uint32_t bytesPerPixel, uint32_t bytesPerScanline)
{
	uint32_t prevIndex = bytePos - bytesPerPixel;
	uint32_t currentScanline = bytePos / bytesPerScanline;
	uint32_t prevScanline = prevIndex / bytesPerScanline;

	if (currentScanline != prevScanline || bytePos % bytesPerScanline == 0)
	{
		prevIndex -= 1;
	}

	if (bytePos >= bytesPerPixel)
	{
		uint8_t newVal = data[bytePos] + data[prevIndex];
		data[bytePos] = newVal;
	}
}

void PNGParser::ReverseAverageFilter(uint8_t * data, uint32_t bytePos, uint32_t bytesPerPixel, uint32_t bytesPerScanLine)
{
	uint32_t leftIndex = bytePos - bytesPerPixel;
	uint32_t left = 0;
	if(left % bytesPerScanLine != 0)
	{
		left = data[leftIndex];
	}

	uint32_t upIndex = bytePos - bytesPerScanLine;
	uint32_t up = 0;
	if(upIndex > 0)
	{
		up = data[upIndex];
	}
	uint32_t average = (left + up) / 2;
	data[bytePos] = data[bytePos] + (uint8_t)average;
}

void PNGParser::ReversePaethFilter(uint8_t * data, uint32_t bytePos, uint32_t bytesPerPixel, uint32_t bytesPerScanLine)
{
	uint32_t leftIndex = bytePos - bytesPerPixel;
	int32_t left = data[leftIndex];
	if (leftIndex % bytesPerScanLine == 0)
	{
		left = 0;
	}

	uint32_t topIndex = bytePos - bytesPerScanLine;
	int32_t top;
	if (topIndex <= 0)
	{
		top = 0;
	}
	else
	{
		top = data[topIndex];
	}

	uint32_t topLeftIndex = leftIndex - bytesPerScanLine;
	int32_t topLeft;
	if(topLeftIndex <=0)
	{
		topLeft = 0;
	}
	else
	{
		topLeft = data[topLeftIndex];
	}
	data[bytePos] = data[bytePos] + (uint8_t)CalculatePeath(left, top, topLeft);
}

uint32_t PNGParser::CalculatePeath(int32_t left, int32_t top, int32_t topLeft)
{
	int32_t p = left + top - topLeft;
	int32_t a = std::abs(p - left);
	int32_t b = std::abs(p - top);
	int32_t c = std::abs(p - topLeft);

	if (a <= b && a <= c)
	{
		return a;
	}
	else if (b <= c)
	{
		return b;
	}
	return c;
}

void PNGParser::ReverseUpFilter(uint8_t *data, uint32_t bytePos, uint32_t bytesPerPixel, uint32_t bytesPerScanline) {
	uint32_t currentScanline = bytePos / bytesPerScanline;
	if (currentScanline > 0)
	{
		data[bytePos] = data[bytePos] + data[bytePos - bytesPerScanline];
	}
}



