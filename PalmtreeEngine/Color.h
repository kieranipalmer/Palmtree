#pragma once
#include <cstdint>

#pragma pack(push, 1)

struct Color {
	float R;
	float G;
	float B;
	float A;

	Color(float R, float G, float B, float A)
	{
		this->R = R;
		this->G = G;
		this->B = B;
		this->A = A;
	}
};

#pragma pack(pop)
