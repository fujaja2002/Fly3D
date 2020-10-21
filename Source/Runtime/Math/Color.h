#pragma once

#include "Runtime/Platform/Platform.h"

struct Color32;
struct LinearColor;

struct LinearColor
{

public:

	static float Pow22OneOver255Table[256];

	static float sRGBToLinearTable[256];

public:

	float r;
	float g;
	float b;
	float a;

public:

	LinearColor();

	LinearColor(float inR, float inG, float inB, float inA = 1.0f);

	LinearColor(const Color32& inColor);

};

struct Color32
{
public:

	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;

public:

	Color32();

	Color32(uint8 inR, uint8 inG, uint8 inB, uint8 inA);

};