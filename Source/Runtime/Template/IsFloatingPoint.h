#pragma once

template <typename T>
struct TIsFloatingPoint
{
	enum 
	{ 
		Value = false 
	};
};

template <> 
struct TIsFloatingPoint<float>       
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsFloatingPoint<double>      
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <> 
struct TIsFloatingPoint<long double> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template <typename T> 
struct TIsFloatingPoint<const T> 
{ 
	enum 
	{ 
		Value = TIsFloatingPoint<T>::Value 
	}; 
};

template <typename T> 
struct TIsFloatingPoint<volatile T> 
{ 
	enum 
	{ 
		Value = TIsFloatingPoint<T>::Value 
	}; 
};

template <typename T> 
struct TIsFloatingPoint<const volatile T> 
{ 
	enum 
	{ 
		Value = TIsFloatingPoint<T>::Value 
	}; 
};
