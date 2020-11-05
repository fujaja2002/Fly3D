#pragma once

#include "Runtime/Platform/Platform.h"

struct FColor32;
struct FLinearColor;

struct FLinearColor
{

public:

	static double Pow22OneOver255Table[256];

	static double sRGBToLinearTable[256];

public:

	float r;
	float g;
	float b;
	float a;

public:

	FLinearColor();

	FLinearColor(float inR, float inG, float inB, float inA = 1.0f);

	FLinearColor(const FColor32& inColor);

};

struct FColor32
{
public:

	uint8 r;
	uint8 g;
	uint8 b;
	uint8 a;

public:

	FColor32();

	FColor32(uint8 inR, uint8 inG, uint8 inB, uint8 inA);

};