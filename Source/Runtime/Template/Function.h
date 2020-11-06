#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Allocator/BaseAllocator.h"

#include "Runtime/Template/AreTypesEqual.h"
#include "Runtime/Template/AndOrNot.h"
#include "Runtime/Template/EnableIf.h"
#include "Runtime/Template/IsMemberPointer.h"
#include "Runtime/Template/IsPointer.h"
#include "Runtime/Template/RemoveReference.h"
#include "Runtime/Template/Decay.h"
#include "Runtime/Template/Invoke.h"
#include "Runtime/Template/IsInvocable.h"
#include "Runtime/Template/IsConstructible.h"
#include "Runtime/Template/RemovePointer.h"
#include "Runtime/Template/Template.h"

template <typename FuncType>
class TFunction;

template <typename FuncType>
class TUniqueFunction;

template <typename FuncType>
class TFunctionRef;

template <typename T> 
struct TIsTFunction               
{ 
	enum 
	{ 
		Value = false 
	}; 
};

template <typename T> 
struct TIsTFunction<TFunction<T>> 
{ 
	enum 
	{ 
		Value = true  
	}; 
};

template <typename T> 
struct TIsTFunction<const T> 
{ 
	enum 
	{ 
		Value = TIsTFunction<T>::Value 
	}; 
};

template <typename T> 
struct TIsTFunction<volatile T> 
{ 
	enum 
	{ 
		Value = TIsTFunction<T>::Value 
	}; 
};

template <typename T> 
struct TIsTFunction<const volatile T> 
{ 
	enum 
	{ 
		Value = TIsTFunction<T>::Value 
	}; 
};

template <typename T> 
struct TIsTUniqueFunction                     
{ 
	enum 
	{ 
		Value = false 
	}; 
};

template <typename T> 
struct TIsTUniqueFunction<TUniqueFunction<T>> 
{ 
	enum 
	{ 
		Value = true  
	}; 
};

template <typename T> 
struct TIsTUniqueFunction<const T> 
{ 
	enum 
	{ 
		Value = TIsTUniqueFunction<T>::Value 
	}; 
};

template <typename T> 
struct TIsTUniqueFunction<volatile T> 
{ 
	enum 
	{ 
		Value = TIsTUniqueFunction<T>::Value 
	}; 
};

template <typename T> 
struct TIsTUniqueFunction<const volatile T> 
{ 
	enum 
	{ 
		Value = TIsTUniqueFunction<T>::Value 
	}; 
};

template <typename T> 
struct TIsTFunctionRef                  
{ 
	enum 
	{ 
		Value = false 
	}; 
};

template <typename T> 
struct TIsTFunctionRef<TFunctionRef<T>> 
{ 
	enum 
	{ 
		Value = true  
	}; 
};

template <typename T> 
struct TIsTFunctionRef<const T> 
{ 
	enum 
	{ 
		Value = TIsTFunctionRef<T>::Value 
	}; 
};

template <typename T> 
struct TIsTFunctionRef<volatile T> 
{ 
	enum 
	{ 
		Value = TIsTFunctionRef<T>::Value 
	}; 
};

template <typename T> 
struct TIsTFunctionRef<const volatile T> 
{ 
	enum 
	{ 
		Value = TIsTFunctionRef<T>::Value 
	}; 
};

namespace Fly3DPrivateFunction
{
	template <typename T>
	struct TFunctionOwnedObject;

	template <bool unique>
	struct TFunctionStorage;

	struct IFunctionOwnedObject
	{
		virtual void* CloneToStorage(void* storage) const = 0;

		virtual void* GetAddress() = 0;

		virtual void Destroy() = 0;

		virtual ~IFunctionOwnedObject() = default;
	};

	template <typename T>
	struct IFunctionOwnedObjectOnHeap : public IFunctionOwnedObject
	{
		virtual void Destroy() override
		{
			void* This = this;
			this->~IFunctionOwnedObjectOnHeap();
			GetAllocator()->Deallocate((uint8*)This);
		}

		~IFunctionOwnedObjectOnHeap() override
		{

		}
	};

	template <typename T>
	struct TFunctionOwnedObject : public IFunctionOwnedObjectOnHeap<T>
	{
		template <typename... ArgTypes>
		explicit TFunctionOwnedObject(ArgTypes&&... args)
			: obj(Forward<ArgTypes>(args)...)
		{

		}

		virtual void* GetAddress() override
		{
			return &obj;
		}

		T obj;
	};

	template <typename T>
	struct TFunctionCopyableOwnedObject final : public TFunctionOwnedObject<T>
	{
		explicit TFunctionCopyableOwnedObject(const T& inObj)
			: TFunctionOwnedObject<T>(inObj)
		{

		}

		explicit TFunctionCopyableOwnedObject(T&& inObj)
			: TFunctionOwnedObject<T>(MoveTemp(inObj))
		{
		}

		void* CloneToStorage(void* inStorage) const override;
	};

	template <typename T>
	struct TFunctionUniqueOwnedObject final : public TFunctionOwnedObject<T>
	{
		explicit TFunctionUniqueOwnedObject(T&& inObj)
			: TFunctionOwnedObject<T>(MoveTemp(inObj))
		{

		}

		void* CloneToStorage(void* storage) const override
		{
			return nullptr;
		}
	};

	template <typename T>
	struct TIsNullableBinding :
		TOr<
			TIsPointer<T>,
			TIsMemberPointer<T>,
			TIsTFunction<T>
		>
	{

	};

	template <typename T>
	FORCE_INLINE typename TEnableIf<TIsNullableBinding<T>::Value, bool>::Type IsBound(const T& func)
	{
		return !!func;
	}

	template <typename T>
	FORCE_INLINE typename TEnableIf<!TIsNullableBinding<T>::Value, bool>::Type IsBound(const T& func)
	{
		return true;
	}

	template <typename FunctorType, bool unique>
	struct TStorageOwnerType;

	template <typename FunctorType>
	struct TStorageOwnerType<FunctorType, true>
	{
		using Type = TFunctionUniqueOwnedObject<typename TDecay<FunctorType>::Type>;
	};

	template <typename FunctorType>
	struct TStorageOwnerType<FunctorType, false>
	{
		using Type = TFunctionCopyableOwnedObject<typename TDecay<FunctorType>::Type>;
	};

	template <typename FunctorType, bool unique>
	using TStorageOwnerTypeT = typename TStorageOwnerType<FunctorType, unique>::Type;

	struct FFunctionStorage
	{
		FFunctionStorage()
			: heapAllocation(nullptr)
		{

		}

		FFunctionStorage(FFunctionStorage&& other)
			: heapAllocation(other.heapAllocation)
		{
			other.heapAllocation = nullptr;
		}

		FFunctionStorage(const FFunctionStorage& other) = delete;

		FFunctionStorage& operator=(FFunctionStorage&& other) = delete;

		FFunctionStorage& operator=(const FFunctionStorage& other) = delete;

		void* BindCopy(const FFunctionStorage& other)
		{
			void* newObj = other.GetBoundObject()->CloneToStorage(this);
			return newObj;
		}

		IFunctionOwnedObject* GetBoundObject() const
		{
			IFunctionOwnedObject* result = (IFunctionOwnedObject*)heapAllocation;
			return result;
		}

		void* GetPtr() const
		{
			return ((IFunctionOwnedObject*)heapAllocation)->GetAddress();
		}

		void Unbind()
		{
			IFunctionOwnedObject* owned = GetBoundObject();
			owned->Destroy();
		}

		void* heapAllocation;
	};

	template <bool unique>
	struct TFunctionStorage : FFunctionStorage
	{
		TFunctionStorage() = default;

		TFunctionStorage(FFunctionStorage&& other)
			: FFunctionStorage(MoveTemp(other))
		{

		}

		template <typename FunctorType>
		typename TDecay<FunctorType>::Type* Bind(FunctorType&& inFunc)
		{
			if (!IsBound(inFunc))
			{
				return nullptr;
			}

			using OwnedType = TStorageOwnerTypeT<FunctorType, unique>;

			void* newAlloc = GetAllocator()->Allocate(sizeof(OwnedType), alignof(OwnedType), EAllocatorType::kMemTypeFunction, __FILE__, __LINE__);
			auto* newOwned = new (newAlloc) OwnedType(Forward<FunctorType>(inFunc));

			heapAllocation = newAlloc;

			return &newOwned->obj;
		}
	};

	template <typename T>
	void* TFunctionCopyableOwnedObject<T>::CloneToStorage(void* inStorage) const
	{
		TFunctionStorage<false>& storage = *(TFunctionStorage<false>*)inStorage;

		void* newAlloc = GetAllocator()->Allocate(sizeof(TFunctionCopyableOwnedObject), alignof(TFunctionCopyableOwnedObject), EAllocatorType::kMemTypeFunction, __FILE__, __LINE__);
		storage.heapAllocation = newAlloc;

		auto* newOwned = new (newAlloc) TFunctionCopyableOwnedObject(this->obj);

		return &newOwned->obj;
	}

	template <typename Functor, typename FuncType>
	struct TFunctionRefCaller;

	template <typename Functor, typename Ret, typename... ParamTypes>
	struct TFunctionRefCaller<Functor, Ret (ParamTypes...)>
	{
		static Ret Call(void* obj, ParamTypes&... params)
		{
			return Invoke(*(Functor*)obj, Forward<ParamTypes>(params)...);
		}
	};

	template <typename Functor, typename... ParamTypes>
	struct TFunctionRefCaller<Functor, void (ParamTypes...)>
	{
		static void Call(void* obj, ParamTypes&... params)
		{
			Invoke(*(Functor*)obj, Forward<ParamTypes>(params)...);
		}
	};

	template <typename StorageType, typename FuncType>
	struct TFunctionRefBase;

	template <typename StorageType, typename Ret, typename... ParamTypes>
	struct TFunctionRefBase<StorageType, Ret (ParamTypes...)>
	{
		template <typename OtherStorageType, typename OtherFuncType>
		friend struct TFunctionRefBase;

		TFunctionRefBase()
			: callable(nullptr)
		{

		}

		TFunctionRefBase(TFunctionRefBase&& other)
			: callable(other.callable)
			, storage (MoveTemp(other.storage))
		{
			if (callable)
			{
				other.callable = nullptr;
			}
		}

		template <typename OtherStorage>
		TFunctionRefBase(TFunctionRefBase<OtherStorage, Ret (ParamTypes...)>&& other)
			: callable(other.callable)
			, storage (MoveTemp(other.storage))
		{
			if (callable)
			{
				other.callable = nullptr;
			}
		}

		template <typename OtherStorage>
		TFunctionRefBase(const TFunctionRefBase<OtherStorage, Ret (ParamTypes...)>& other)
			: callable(other.callable)
		{
			if (callable)
			{
				storage.BindCopy(other.storage);
			}
		}

		TFunctionRefBase(const TFunctionRefBase& other)
			: callable(other.callable)
		{
			if (callable)
			{
				storage.BindCopy(other.storage);
			}
		}

		template <
			typename FunctorType,
			typename = typename TEnableIf<
				TNot<
					TIsSame<TFunctionRefBase, typename TDecay<FunctorType>::Type>
				>::Value
			>::Type
		>
		TFunctionRefBase(FunctorType&& inFunc)
		{
			if (auto* Binding = storage.Bind(Forward<FunctorType>(inFunc)))
			{
				using DecayedFunctorType = typename TRemovePointer<decltype(Binding)>::Type;
				callable = &TFunctionRefCaller<DecayedFunctorType, Ret (ParamTypes...)>::Call;
			}
		}

		TFunctionRefBase& operator=(TFunctionRefBase&&) = delete;

		TFunctionRefBase& operator=(const TFunctionRefBase&) = delete;

		void CheckCallable() const
		{
			Assert(callable);
		}

		Ret operator()(ParamTypes... params) const
		{
			CheckCallable();
			return callable(storage.GetPtr(), params...);
		}

		~TFunctionRefBase()
		{
			if (callable)
			{
				storage.Unbind();
			}
		}

	protected:

		bool IsSet() const
		{
			return !!callable;
		}

	private:

		Ret (*callable)(void*, ParamTypes&...);

		StorageType storage;
	};

	template <typename FunctorType, typename Ret, typename... ParamTypes>
	struct TFunctorReturnTypeIsCompatible : TIsConstructible<Ret, decltype(DeclVal<FunctorType>()(DeclVal<ParamTypes>()...))>
	{

	};

	template <typename MemberRet, typename Class, typename Ret, typename... ParamTypes>
	struct TFunctorReturnTypeIsCompatible<MemberRet Class::*, Ret, ParamTypes...> : TIsConstructible<Ret, MemberRet>
	{

	};

	template <typename MemberRet, typename Class, typename Ret, typename... ParamTypes>
	struct TFunctorReturnTypeIsCompatible<MemberRet Class::* const, Ret, ParamTypes...> : TIsConstructible<Ret, MemberRet>
	{

	};

	template <typename MemberRet, typename Class, typename... MemberParamTypes, typename Ret, typename... ParamTypes>
	struct TFunctorReturnTypeIsCompatible<MemberRet (Class::*)(MemberParamTypes...), Ret, ParamTypes...> : TIsConstructible<Ret, MemberRet>
	{

	};

	template <typename MemberRet, typename Class, typename... MemberParamTypes, typename Ret, typename... ParamTypes>
	struct TFunctorReturnTypeIsCompatible<MemberRet (Class::*)(MemberParamTypes...) const, Ret, ParamTypes...> : TIsConstructible<Ret, MemberRet>
	{

	};

	template <typename FuncType, typename FunctorType>
	struct TFuncCanBindToFunctor;

	template <typename FunctorType, typename Ret, typename... ParamTypes>
	struct TFuncCanBindToFunctor<Ret(ParamTypes...), FunctorType> : TAnd<TIsInvocable<FunctorType, ParamTypes...>, TFunctorReturnTypeIsCompatible<FunctorType, Ret, ParamTypes...>>
	{

	};

	template <typename FunctorType, typename... ParamTypes>
	struct TFuncCanBindToFunctor<void(ParamTypes...), FunctorType> : TIsInvocable<FunctorType, ParamTypes...>
	{

	};

	struct FFunctionRefStoragePolicy
	{
		template <typename FunctorType>
		typename TRemoveReference<FunctorType>::Type* Bind(FunctorType&& inFunc)
		{
			Assert(IsBound(inFunc));

			ptr = (void*)&inFunc;
			return &inFunc;
		}

		void* BindCopy(const FFunctionRefStoragePolicy& other)
		{
			void* otherPtr = other.ptr;
			ptr = otherPtr;
			return otherPtr;
		}

		void* GetPtr() const
		{
			return ptr;
		}

		void Unbind() const
		{

		}

	private:

		void* ptr;
	};
}

template <typename FuncType>
class TFunctionRef : public Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::FFunctionRefStoragePolicy, FuncType>
{
	using Super = Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::FFunctionRefStoragePolicy, FuncType>;

public:
	
	template <
		typename FunctorType,
		typename = typename TEnableIf<
			TAnd<
				TNot<TIsTFunctionRef<typename TDecay<FunctorType>::Type>>,
				Fly3DPrivateFunction::TFuncCanBindToFunctor<FuncType, typename TDecay<FunctorType>::Type>
			>::Value
		>::Type
	>
	TFunctionRef(FunctorType&& inFunc)
		: Super(Forward<FunctorType>(inFunc))
	{

	}

	TFunctionRef(const TFunctionRef&) = default;

	TFunctionRef& operator=(const TFunctionRef&) const = delete;

	~TFunctionRef() = default;
};

template <typename FuncType>
class TFunction final : public Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::TFunctionStorage<false>, FuncType>
{
	using Super = Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::TFunctionStorage<false>, FuncType>;

public:

	TFunction(TYPE_OF_NULLPTR = nullptr)
	{

	}

	template <
		typename FunctorType,
		typename = typename TEnableIf<
			TAnd<
				TNot<TIsTFunction<typename TDecay<FunctorType>::Type>>,
				Fly3DPrivateFunction::TFuncCanBindToFunctor<FuncType, FunctorType>
			>::Value
		>::Type
	>
	TFunction(FunctorType&& inFunc)
		: Super(Forward<FunctorType>(inFunc))
	{
		static_assert(!TIsTFunctionRef<typename TDecay<FunctorType>::Type>::Value, "Cannot construct a TFunction from a TFunctionRef");
	}

	TFunction(TFunction&&) = default;

	TFunction(const TFunction& other) = default;

	~TFunction() = default;

	TFunction& operator=(TFunction&& other)
	{
		Swap(*this, other);
		return *this;
	}

	TFunction& operator=(const TFunction& other)
	{
		TFunction temp = other;
		Swap(*this, temp);
		return *this;
	}

	FORCE_INLINE explicit operator bool() const
	{
		return Super::IsSet();
	}
};

template <typename FuncType>
class TUniqueFunction final : public Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::TFunctionStorage<true>, FuncType>
{
	using Super = Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::TFunctionStorage<true>, FuncType>;

public:

	TUniqueFunction(TYPE_OF_NULLPTR = nullptr)
	{

	}

	template <
		typename FunctorType,
		typename = typename TEnableIf<
			TAnd<
				TNot<TOr<TIsTUniqueFunction<typename TDecay<FunctorType>::Type>, TIsTFunction<typename TDecay<FunctorType>::Type>>>,
				Fly3DPrivateFunction::TFuncCanBindToFunctor<FuncType, FunctorType>
			>::Value
		>::Type
	>
	TUniqueFunction(FunctorType&& inFunc)
		: Super(Forward<FunctorType>(inFunc))
	{
		static_assert(!TIsTFunctionRef<typename TDecay<FunctorType>::Type>::Value, "Cannot construct a TUniqueFunction from a TFunctionRef");
	}

	TUniqueFunction(TFunction<FuncType>&& other)
		: Super(MoveTemp(*(Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::TFunctionStorage<false>, FuncType>*)&other))
	{

	}

	TUniqueFunction(const TFunction<FuncType>& other)
		: Super(*(const Fly3DPrivateFunction::TFunctionRefBase<Fly3DPrivateFunction::TFunctionStorage<false>, FuncType>*)&other)
	{

	}

	TUniqueFunction& operator=(TUniqueFunction&& other)
	{
		Swap(*this, other);
		return *this;
	}

	TUniqueFunction(TUniqueFunction&&) = default;

	TUniqueFunction(const TUniqueFunction& other) = delete;

	TUniqueFunction& operator=(const TUniqueFunction& other) = delete;

	~TUniqueFunction() = default;

	FORCE_INLINE explicit operator bool() const
	{
		return Super::IsSet();
	}
};

template <typename FuncType>
FORCE_INLINE bool operator==(TYPE_OF_NULLPTR, const TFunction<FuncType>& func)
{
	return !func;
}

template <typename FuncType>
FORCE_INLINE bool operator==(const TFunction<FuncType>& func, TYPE_OF_NULLPTR)
{
	return !func;
}

template <typename FuncType>
FORCE_INLINE bool operator!=(TYPE_OF_NULLPTR, const TFunction<FuncType>& func)
{
	return (bool)func;
}

template <typename FuncType>
FORCE_INLINE bool operator!=(const TFunction<FuncType>& func, TYPE_OF_NULLPTR)
{
	return (bool)func;
}