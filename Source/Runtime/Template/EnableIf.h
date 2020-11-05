#pragma once

template <bool predicate, typename Result = void>
class TEnableIf;

template <typename Result>
class TEnableIf<true, Result>
{
public:
	typedef Result Type;
};

template <typename Result>
class TEnableIf<false, Result>
{ 

};

template <bool predicate, typename Func>
class TLazyEnableIf;

template <typename Func>
class TLazyEnableIf<true, Func>
{
public:
	typedef typename Func::Type Type;
};

template <typename Func>
class TLazyEnableIf<false, Func>
{ 

};
