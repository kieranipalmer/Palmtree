#include "BitmapParser.h"



Texture * BitmapParser::ParseBitmap(unsigned char * rawData)
{
	//Texture* retVal = new Texture();
	BitmapHeader* header = (BitmapHeader*)rawData;
	BitmapInfoHeader* infoHeader = (BitmapInfoHeader*)(rawData + 14);

	return nullptr;
}
	