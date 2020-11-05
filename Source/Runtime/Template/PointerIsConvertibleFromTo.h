#pragma once

#include "Runtime/Platform/Platform.h"

template <typename From, typename To>
struct TPointerIsConvertibleFromTo
{
private:
	static uint8  Test(...);

	static uint16 Test(To*);

public:
	enum 
	{ 
		Value  = sizeof(Test((From*)nullptr)) - 1 
	};
};

class TPointerIsConvertibleFromToTestBase
{

};

class TPointerIsConvertibleFromToTestDerived : public TPointerIsConvertibleFromToTestBase
{

};

class TPointerIsConvertibleFromToUnrelated
{

};