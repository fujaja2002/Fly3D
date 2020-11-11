#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Template/EnableIf.h"
#include "Runtime/Template/IsTriviallyDestructible.h"

template <typename ElementType>
FORCE_INLINE typename TEnableIf<!TIsTriviallyDestructible<ElementType>::Value>::Type DestructItem(ElementType* element)
{
	typedef ElementType DestructItemsElementTypeTypedef;
	element->DestructItemsElementTypeTypedef::~DestructItemsElementTypeTypedef();
}

template <typename ElementType>
FORCE_INLINE typename TEnableIf<TIsTriviallyDestructible<ElementType>::Value>::Type DestructItem(ElementType* element)
{

}