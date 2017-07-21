#pragma once
#include <cstdint>
class EdianessUtility {
public:

	static bool IsLittleEndian()
	{
		//Check Endianess of platform
		volatile uint32_t i = 0x00000001;
		uint8_t* firstBytePointer = (uint8_t*)(&i);
		return ((*firstBytePointer) == 0x01);
	}

	static inline void SwapEndian(uint32_t& val)
	{
		val = (val << 24) | ((val << 8) & 0x00ff0000) | ((val >> 8) & 0x0000ff00) | (val >> 24);
	}

	static inline void SwapEndian(uint16_t& val)
	{
		val = (val << 8) | (val >> 8);
	}
};