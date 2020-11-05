#pragma once

template <typename T, typename... Args>
struct TIsConstructible
{
	enum 
	{ 
		Value = __is_constructible(T, Args...) 
	};
};
