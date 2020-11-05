#pragma once

template <typename T> 
struct TRemovePointer     
{ 
	typedef T Type; 
};

template <typename T> 
struct TRemovePointer<T*> 
{
	typedef T Type; 
};