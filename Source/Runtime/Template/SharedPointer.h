#pragma once

#include "Runtime/Platform/Platform.h"
#include "Runtime/Core/PlatformAtomics.h"
#include "Runtime/Core/PlatformMemory.h"
#include "Runtime/Template/AndOrNot.h"
#include "Runtime/Template/RemoveReference.h"
#include "Runtime/Template/TypeCompatibleBytes.h"
#include "Runtime/Template/PointerIsConvertibleFromTo.h"
#include "Runtime/Template/EnableIf.h"
#include "Runtime/Template/TypeTraits.h"
#include "Runtime/Template/MemoryOps.h"
#include "Runtime/Template/Template.h"

enum class ESPMode
{
	ThreadUnSafe = 0,
	ThreadSafe   = 1
};

template<class ObjectType, ESPMode Mode = ESPMode::ThreadUnSafe> 
class TSharedRef;

template<class ObjectType, ESPMode Mode = ESPMode::ThreadUnSafe> 
class TSharedPtr;

template<class ObjectType, ESPMode Mode = ESPMode::ThreadUnSafe> 
class TWeakPtr;

template<class ObjectType, ESPMode Mode = ESPMode::ThreadUnSafe> 
class TSharedFromThis;

namespace Fly3DPrivateSharedPointer
{
	template<ESPMode Mode> 
	class FWeakReferencer;

	struct FNullTag 
	{

	};

	struct FConstCastTag 
	{

	};

	struct FStaticCastTag 
	{

	};

	class FReferenceControllerBase
	{
	public:

		explicit FReferenceControllerBase()
			: sharedReferenceCount(1)
			, weakReferenceCount(1)
		{

		}

		virtual ~FReferenceControllerBase()
		{

		}

		virtual void DestroyObject() = 0;

	public:

		int32 sharedReferenceCount;
		int32 weakReferenceCount;

	private:

		FReferenceControllerBase(FReferenceControllerBase const& controller);

		FReferenceControllerBase& operator=(FReferenceControllerBase const& controller);
	};

	template <typename ObjectType, typename DeleterType>
	class TReferenceControllerWithDeleter : private DeleterType, public FReferenceControllerBase
	{
	public:
		explicit TReferenceControllerWithDeleter(ObjectType* inObject, DeleterType&& deleter)
			: DeleterType(MoveTemp(deleter))
			, object(inObject)
		{

		}

		virtual void DestroyObject() override
		{
			(*static_cast<DeleterType*>(this))(object);
		}

		TReferenceControllerWithDeleter(const TReferenceControllerWithDeleter& controller) = delete;

		TReferenceControllerWithDeleter& operator=(const TReferenceControllerWithDeleter& controller) = delete;

	private:
		ObjectType* object;
	};

	template <typename ObjectType>
	class TIntrusiveReferenceController : public FReferenceControllerBase
	{
	public:

		template <typename... ArgTypes>
		explicit TIntrusiveReferenceController(ArgTypes&&... args)
		{
			new ((void*)&objectStorage) ObjectType(Forward<ArgTypes>(args)...);
		}

		ObjectType* GetObjectPtr() const
		{
			return (ObjectType*)&objectStorage;
		}

		virtual void DestroyObject() override
		{
			DestructItem((ObjectType*)&objectStorage);
		}

		TIntrusiveReferenceController(const TIntrusiveReferenceController& controller) = delete;

		TIntrusiveReferenceController& operator=(const TIntrusiveReferenceController& controller) = delete;

	private:
		mutable TTypeCompatibleBytes<ObjectType> objectStorage;
	};

	template <typename Type>
	struct DefaultDeleter
	{
		FORCE_INLINE void operator()(Type* object) const
		{
			delete object;
		}
	};

	template <typename ObjectType>
	FORCE_INLINE FReferenceControllerBase* NewDefaultReferenceController(ObjectType* object)
	{
		return new TReferenceControllerWithDeleter<ObjectType, DefaultDeleter<ObjectType>>(object, DefaultDeleter<ObjectType>());
	}

	template <typename ObjectType, typename DeleterType>
	FORCE_INLINE FReferenceControllerBase* NewCustomReferenceController(ObjectType* object, DeleterType&& deleter)
	{
		return new TReferenceControllerWithDeleter<ObjectType, typename TRemoveReference<DeleterType>::Type>(object, Forward<DeleterType>(deleter));
	}

	template <typename ObjectType, typename... ArgTypes>
	FORCE_INLINE TIntrusiveReferenceController<ObjectType>* NewIntrusiveReferenceController(ArgTypes&&... args)
	{
		return new TIntrusiveReferenceController<ObjectType>(Forward<ArgTypes>(args)...);
	}

	template<class ObjectType>
	struct FRawPtrProxy
	{
		FRawPtrProxy(ObjectType* inObject)
			: object(inObject)
			, referenceController(NewDefaultReferenceController(inObject))
		{

		}

		template<class deleter>
		FRawPtrProxy(ObjectType* inObject, deleter&& inDeleter)
			: object(inObject)
			, referenceController(NewCustomReferenceController(inObject, Forward<deleter>(inDeleter)))
		{

		}

		ObjectType* object;
		FReferenceControllerBase* referenceController;
	};

	template<ESPMode Mode>
	struct FReferenceControllerOps;

	template<>
	struct FReferenceControllerOps<ESPMode::ThreadSafe>
	{
		static FORCE_INLINE const int32 GetSharedReferenceCount(const FReferenceControllerBase* referenceController)
		{
			return FPlatformAtomics::AtomicRead((int32 volatile*)&referenceController->sharedReferenceCount);
		}

		static FORCE_INLINE void AddSharedReference(FReferenceControllerBase* referenceController)
		{
			FPlatformAtomics::InterlockedIncrement(&referenceController->sharedReferenceCount);
		}

		static bool ConditionallyAddSharedReference(FReferenceControllerBase* referenceController)
		{
			while (true)
			{
				const int32 originalCount = FPlatformAtomics::AtomicRead((int32 volatile*)&referenceController->sharedReferenceCount);
				if (originalCount == 0)
				{
					return false;
				}

				const int32 actualOriginalCount = FPlatformAtomics::InterlockedCompareExchange(&referenceController->sharedReferenceCount, originalCount + 1, originalCount);
				if (actualOriginalCount == originalCount)
				{
					return true;
				}
			}
		}

		static FORCE_INLINE void ReleaseSharedReference(FReferenceControllerBase* referenceController)
		{
			if (FPlatformAtomics::InterlockedDecrement(&referenceController->sharedReferenceCount) == 0)
			{
				referenceController->DestroyObject();
				ReleaseWeakReference(referenceController);
			}
		}

		static FORCE_INLINE void AddWeakReference(FReferenceControllerBase* referenceController)
		{
			FPlatformAtomics::InterlockedIncrement(&referenceController->weakReferenceCount);
		}

		static void ReleaseWeakReference(FReferenceControllerBase* referenceController)
		{
			if (FPlatformAtomics::InterlockedDecrement(&referenceController->weakReferenceCount) == 0)
			{
				delete referenceController;
			}
		}
	};

	template<>
	struct FReferenceControllerOps<ESPMode::ThreadUnSafe>
	{
		static FORCE_INLINE const int32 GetSharedReferenceCount(const FReferenceControllerBase* referenceController)
		{
			return referenceController->sharedReferenceCount;
		}

		static FORCE_INLINE void AddSharedReference(FReferenceControllerBase* referenceController)
		{
			++referenceController->sharedReferenceCount;
		}

		static bool ConditionallyAddSharedReference(FReferenceControllerBase* referenceController)
		{
			if (referenceController->sharedReferenceCount == 0)
			{
				return false;
			}

			++referenceController->sharedReferenceCount;
			return true;
		}

		static FORCE_INLINE void ReleaseSharedReference(FReferenceControllerBase* referenceController)
		{
			if (--referenceController->sharedReferenceCount == 0)
			{
				referenceController->DestroyObject();
				ReleaseWeakReference(referenceController);
			}
		}

		static FORCE_INLINE void AddWeakReference(FReferenceControllerBase* referenceController)
		{
			++referenceController->weakReferenceCount;
		}

		static void ReleaseWeakReference(FReferenceControllerBase* referenceController)
		{
			if (--referenceController->weakReferenceCount == 0)
			{
				delete referenceController;
			}
		}
	};

	template<ESPMode Mode>
	class FSharedReferencer
	{
		typedef FReferenceControllerOps<Mode> TOps;

	public:

		FSharedReferencer()
			: referenceController(nullptr)
		{ 

		}

		explicit FSharedReferencer(FReferenceControllerBase* inReferenceController)
			: referenceController(inReferenceController)
		{ 

		}

		FSharedReferencer(FSharedReferencer const& inSharedReference)
			: referenceController(inSharedReference.referenceController)
		{
			if (referenceController != nullptr)
			{
				TOps::AddSharedReference(referenceController);
			}
		}

		FSharedReferencer(FSharedReferencer&& inSharedReference)
			: referenceController(inSharedReference.referenceController)
		{
			inSharedReference.referenceController = nullptr;
		}

		FSharedReferencer(FWeakReferencer<Mode> const& inWeakReference)
			: referenceController(inWeakReference.referenceController)
		{
			if (referenceController != nullptr)
			{
				if (!TOps::ConditionallyAddSharedReference(referenceController))
				{
					referenceController = nullptr;
				}
			}
		}

		FSharedReferencer(FWeakReferencer<Mode>&& inWeakReference)
			: referenceController(inWeakReference.referenceController)
		{
			if (referenceController != nullptr)
			{
				if (!TOps::ConditionallyAddSharedReference(referenceController))
				{
					referenceController = nullptr;
				}

				TOps::ReleaseWeakReference(inWeakReference.referenceController);
				inWeakReference.referenceController = nullptr;
			}
		}

		~FSharedReferencer()
		{
			if (referenceController != nullptr )
			{
				TOps::ReleaseSharedReference(referenceController);
			}
		}

		FSharedReferencer& operator=(FSharedReferencer const& inSharedReference)
		{
			auto newReferenceController = inSharedReference.referenceController;

			if (newReferenceController != referenceController)
			{
				if (newReferenceController != nullptr)
				{
					TOps::AddSharedReference(newReferenceController);
				}

				if (referenceController != nullptr)
				{
					TOps::ReleaseSharedReference(referenceController);
				}

				referenceController = newReferenceController;
			}

			return *this;
		}

		FSharedReferencer& operator=(FSharedReferencer&& inSharedReference)
		{
			auto newReferenceController = inSharedReference.referenceController;
			auto oldReferenceController = referenceController;

			if (newReferenceController != oldReferenceController)
			{
				inSharedReference.referenceController = nullptr;
				referenceController = newReferenceController;

				if (oldReferenceController != nullptr)
				{
					TOps::ReleaseSharedReference(oldReferenceController);
				}
			}

			return *this;
		}

		FORCE_INLINE const bool IsValid() const
		{
			return referenceController != nullptr;
		}

		FORCE_INLINE const int32 GetSharedReferenceCount() const
		{
			return referenceController != nullptr ? TOps::GetSharedReferenceCount(referenceController) : 0;
		}

		FORCE_INLINE const bool IsUnique() const
		{
			return GetSharedReferenceCount() == 1;
		}

	private:

		template<ESPMode OtherMode> 
		friend class FWeakReferencer;

	private:

		FReferenceControllerBase* referenceController;
	};

	template<ESPMode Mode>
	class FWeakReferencer
	{
		typedef FReferenceControllerOps<Mode> TOps;

	public:

		FWeakReferencer()
			: referenceController(nullptr)
		{ 

		}

		FWeakReferencer(FWeakReferencer const& inWeakRefCountPointer)
			: referenceController(inWeakRefCountPointer.referenceController)
		{
			if (referenceController != nullptr)
			{
				TOps::AddWeakReference(referenceController);
			}
		}

		FWeakReferencer(FWeakReferencer&& inWeakRefCountPointer)
			: referenceController(inWeakRefCountPointer.referenceController)
		{
			inWeakRefCountPointer.referenceController = nullptr;
		}

		FWeakReferencer(FSharedReferencer<Mode> const& inSharedRefCountPointer)
			: referenceController(inSharedRefCountPointer.referenceController)
		{
			if (referenceController != nullptr)
			{
				TOps::AddWeakReference(referenceController);
			}
		}

		~FWeakReferencer()
		{
			if (referenceController != nullptr)
			{
				TOps::ReleaseWeakReference(referenceController);
			}
		}

		FWeakReferencer& operator=(FWeakReferencer const& inWeakReference)
		{
			AssignReferenceController(inWeakReference.referenceController);
			return *this;
		}

		FWeakReferencer& operator=(FWeakReferencer&& inWeakReference)
		{
			auto oldReferenceController = referenceController;
			referenceController = inWeakReference.referenceController;
			inWeakReference.referenceController = nullptr;

			if (oldReferenceController != nullptr)
			{
				TOps::ReleaseWeakReference(oldReferenceController);
			}

			return *this;
		}

		FWeakReferencer& operator=(FSharedReferencer<Mode> const& inSharedReference)
		{
			AssignReferenceController(inSharedReference.referenceController);

			return *this;
		}

		const bool IsValid() const
		{
			return referenceController != nullptr && TOps::GetSharedReferenceCount(referenceController)> 0;
		}

	private:

		void AssignReferenceController(FReferenceControllerBase* newReferenceController)
		{
			if (newReferenceController != referenceController)
			{
				if (newReferenceController != nullptr)
				{
					TOps::AddWeakReference(newReferenceController);
				}

				if (referenceController != nullptr)
				{
					TOps::ReleaseWeakReference(referenceController);
				}

				referenceController = newReferenceController;
			}
		}

	private:

		template<ESPMode OtherMode> 
		friend class FSharedReferencer;

	private:

		FReferenceControllerBase* referenceController;
	};

	template<class SharedPtrType, class ObjectType, class OtherType, ESPMode Mode>
	FORCE_INLINE void EnableSharedFromThis(TSharedPtr<SharedPtrType, Mode> const* inSharedPtr, ObjectType const* inObject, TSharedFromThis<OtherType, Mode> const* inShareable)
	{
		if (inShareable != nullptr)
		{
			inShareable->UpdateWeakReferenceInternal(inSharedPtr, const_cast<ObjectType*>(inObject));
		}
	}

	template<class SharedPtrType, class ObjectType, class OtherType, ESPMode Mode>
	FORCE_INLINE void EnableSharedFromThis(TSharedPtr<SharedPtrType, Mode>* inSharedPtr, ObjectType const* inObject, TSharedFromThis<OtherType, Mode> const* inShareable)
	{
		if (inShareable != nullptr)
		{
			inShareable->UpdateWeakReferenceInternal(inSharedPtr, const_cast<ObjectType*>(inObject));
		}
	}

	template<class SharedRefType, class ObjectType, class OtherType, ESPMode Mode>
	FORCE_INLINE void EnableSharedFromThis(TSharedRef<SharedRefType, Mode> const* inSharedRef, ObjectType const* inObject, TSharedFromThis<OtherType, Mode> const* inShareable)
	{
		if (inShareable != nullptr)
		{
			inShareable->UpdateWeakReferenceInternal(inSharedRef, const_cast<ObjectType*>(inObject));
		}
	}

	template<class SharedRefType, class ObjectType, class OtherType, ESPMode Mode>
	FORCE_INLINE void EnableSharedFromThis(TSharedRef<SharedRefType, Mode>* inSharedRef, ObjectType const* inObject, TSharedFromThis<OtherType, Mode> const* inShareable)
	{
		if (inShareable != nullptr)
		{
			inShareable->UpdateWeakReferenceInternal(inSharedRef, const_cast<ObjectType*>(inObject));
		}
	}

	FORCE_INLINE void EnableSharedFromThis( ... ) 
	{

	}

	template <typename ObjectType, ESPMode Mode>
	FORCE_INLINE TSharedRef<ObjectType, Mode> MakeSharedRef(ObjectType* inObject, FReferenceControllerBase* inSharedReferenceCount)
	{
		return TSharedRef<ObjectType, Mode>(inObject, inSharedReferenceCount);
	}
}

template<class CastToType, class CastFromType, ESPMode Mode>
FORCE_INLINE TSharedRef<CastToType, Mode> StaticCastSharedRef(TSharedRef<CastFromType, Mode> const& inSharedRef)
{
	return TSharedRef<CastToType, Mode>(inSharedRef, Fly3DPrivateSharedPointer::FStaticCastTag());
}

template<class ObjectType, ESPMode Mode>
class TSharedRef
{
public:

	using ElementType = ObjectType;

	template <
		typename OtherType, 
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	explicit TSharedRef(OtherType* inObject)
		: object(inObject)
		, sharedReferenceCount(Fly3DPrivateSharedPointer::NewDefaultReferenceController(inObject))
	{
		Init(inObject);
	}

	template <
		typename OtherType,
		typename DeleterType,
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	TSharedRef(OtherType* inObject, DeleterType&& inDeleter)
		: object(inObject)
		, sharedReferenceCount(Fly3DPrivateSharedPointer::NewCustomReferenceController(inObject, Forward<DeleterType>(inDeleter)))
	{
		Init(inObject);
	}

	TSharedRef()
		: object(new ObjectType())
		, sharedReferenceCount(Fly3DPrivateSharedPointer::NewDefaultReferenceController(object))
	{
		Init(object);
	}

	template <
		typename OtherType,
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	TSharedRef(Fly3DPrivateSharedPointer::FRawPtrProxy<OtherType> const& inRawPtrProxy)
		: object(inRawPtrProxy.object)
		, sharedReferenceCount(inRawPtrProxy.ReferenceController)
	{
		Fly3DPrivateSharedPointer::EnableSharedFromThis(this, inRawPtrProxy.object, inRawPtrProxy.object);
	}

	template <
		typename OtherType,
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	TSharedRef(TSharedRef<OtherType, Mode> const& inSharedRef)
		: object(inSharedRef.object)
		, sharedReferenceCount(inSharedRef.sharedReferenceCount)
	{

	}

	template <typename OtherType>
	TSharedRef(TSharedRef<OtherType, Mode> const& inSharedRef, Fly3DPrivateSharedPointer::FStaticCastTag tag)
		: object(static_cast<ObjectType*>(inSharedRef.object))
		, sharedReferenceCount(inSharedRef.sharedReferenceCount)
	{

	}

	template <typename OtherType>
	TSharedRef(TSharedRef<OtherType, Mode> const& inSharedRef, Fly3DPrivateSharedPointer::FConstCastTag tag)
		: object(const_cast<ObjectType*>(inSharedRef.object))
		, sharedReferenceCount(inSharedRef.sharedReferenceCount)
	{

	}

	template <typename OtherType>
	TSharedRef(TSharedRef<OtherType, Mode> const& otherSharedRef, ObjectType* inObject)
		: object(inObject)
		, sharedReferenceCount(otherSharedRef.sharedReferenceCount)
	{

	}

	TSharedRef(TSharedRef const& inSharedRef)
		: object(inSharedRef.object)
		, sharedReferenceCount(inSharedRef.sharedReferenceCount)
	{ 

	}

	TSharedRef(TSharedRef&& inSharedRef)
		: object(inSharedRef.object)
		, sharedReferenceCount(inSharedRef.sharedReferenceCount)
	{

	}

	TSharedRef& operator=(TSharedRef const& inSharedRef)
	{
		TSharedRef temp = inSharedRef;
		Swap(temp, *this);
		return *this;
	}

	FORCE_INLINE TSharedRef& operator=(TSharedRef&& inSharedRef)
	{
		FPlatformMemory::Memswap(this, &inSharedRef, sizeof(TSharedRef));
		return *this;
	}

	template <
		typename OtherType,
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	TSharedRef& operator=(Fly3DPrivateSharedPointer::FRawPtrProxy<OtherType> const& inRawPtrProxy)
	{
		*this = TSharedRef<ObjectType, Mode>(inRawPtrProxy);
		return *this;
	}

	FORCE_INLINE ObjectType& Get() const
	{
		return *object;
	}

	FORCE_INLINE ObjectType& operator*() const
	{
		return *object;
	}

	FORCE_INLINE ObjectType* operator->() const
	{
		return object;
	}

	FORCE_INLINE const int32 GetSharedReferenceCount() const
	{
		return sharedReferenceCount.GetSharedReferenceCount();
	}

	FORCE_INLINE const bool IsUnique() const
	{
		return sharedReferenceCount.IsUnique();
	}

private:

	template<class OtherType>
	void Init(OtherType* inObject)
	{
		Fly3DPrivateSharedPointer::EnableSharedFromThis(this, inObject, inObject);
	}

	template <
		typename OtherType,
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	explicit TSharedRef(TSharedPtr<OtherType, Mode> const& inSharedPtr)
		: object(inSharedPtr.object)
		, sharedReferenceCount(inSharedPtr.sharedReferenceCount)
	{

	}

	template <
		typename OtherType,
		typename = decltype(ImplicitConv<ObjectType*>((OtherType*)nullptr))
	>
	explicit TSharedRef(TSharedPtr<OtherType, Mode>&& inSharedPtr)
		: object(inSharedPtr.object)
		, sharedReferenceCount(MoveTemp(inSharedPtr.sharedReferenceCount))
	{
		inSharedPtr.object = nullptr;
	}

	FORCE_INLINE const bool IsValid() const
	{
		return object != nullptr;
	}

	template<class OtherType, ESPMode OtherMode> 
	friend class TSharedRef;

	template<class OtherType, ESPMode OtherMode> 
	friend class TSharedPtr;

	template<class OtherType, ESPMode OtherMode> 
	friend class TWeakPtr;

private:
	explicit TSharedRef(ObjectType* inObject, Fly3DPrivateSharedPointer::FReferenceControllerBase* inSharedReferenceCount)
		: object(inObject)
		, sharedReferenceCount(inSharedReferenceCount)
	{
		Init(inObject);
	}

private:
	friend TSharedRef Fly3DPrivateSharedPointer::MakeSharedRef<ObjectType, Mode>(ObjectType* inObject, Fly3DPrivateSharedPointer::FReferenceControllerBase* inSharedReferenceCount);

private:

	ObjectType* object;

	Fly3DPrivateSharedPointer::FSharedReferencer<Mode> sharedReferenceCount;
};

template<class T>
struct FMakeReferenceTo
{
	typedef T& Type;
};

template<>
struct FMakeReferenceTo<void>
{
	typedef void Type;
};

template<>
struct FMakeReferenceTo<const void>
{
	typedef void Type;
};

template<class ObjectType, ESPMode Mode>
class TSharedPtr
{
public:

	using ElementType = ObjectType;
	
	TSharedPtr(Fly3DPrivateSharedPointer::FNullTag* tag = nullptr)
		: object(nullptr)
		, sharedReferenceCount()
	{

	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	explicit TSharedPtr(OtherType* inObject)
		: object(inObject)
		, sharedReferenceCount(Fly3DPrivateSharedPointer::NewDefaultReferenceController(inObject))
	{
		Fly3DPrivateSharedPointer::EnableSharedFromThis(this, inObject, inObject);
	}

	template <
		typename OtherType,
		typename DeleterType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TSharedPtr(OtherType* inObject, DeleterType&& inDeleter)
		: object(inObject)
		, sharedReferenceCount(Fly3DPrivateSharedPointer::NewCustomReferenceController(inObject, Forward<DeleterType>(inDeleter)))
	{
		Fly3DPrivateSharedPointer::EnableSharedFromThis(this, inObject, inObject);
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TSharedPtr(Fly3DPrivateSharedPointer::FRawPtrProxy<OtherType> const& inRawPtrProxy)
		: object(inRawPtrProxy.object )
		, sharedReferenceCount(inRawPtrProxy.ReferenceController)
	{
		Fly3DPrivateSharedPointer::EnableSharedFromThis(this, inRawPtrProxy.object, inRawPtrProxy.object);
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TSharedPtr(TSharedPtr<OtherType, Mode> const& inSharedPtr)
		: object(inSharedPtr.object)
		, sharedReferenceCount(inSharedPtr.sharedReferenceCount)
	{ 

	}

	TSharedPtr(TSharedPtr const& inSharedPtr)
		: object(inSharedPtr.object)
		, sharedReferenceCount(inSharedPtr.sharedReferenceCount)
	{

	}

	TSharedPtr(TSharedPtr&& inSharedPtr)
		: object(inSharedPtr.object)
		, sharedReferenceCount(MoveTemp(inSharedPtr.sharedReferenceCount))
	{
		inSharedPtr.object = nullptr;
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TSharedPtr(TSharedRef<OtherType, Mode> const& inSharedRef)
		: object(inSharedRef.object)
		, sharedReferenceCount(inSharedRef.sharedReferenceCount)
	{

	}

	template <typename OtherType>
	TSharedPtr(TSharedPtr<OtherType, Mode> const& inSharedPtr, Fly3DPrivateSharedPointer::FStaticCastTag)
		: object(static_cast<ObjectType*>(inSharedPtr.object))
		, sharedReferenceCount(inSharedPtr.sharedReferenceCount)
	{ 

	}

	template <typename OtherType>
	TSharedPtr(TSharedPtr<OtherType, Mode> const& inSharedPtr, Fly3DPrivateSharedPointer::FConstCastTag)
		: object(const_cast<ObjectType*>(inSharedPtr.object))
		, sharedReferenceCount(inSharedPtr.sharedReferenceCount)
	{ 

	}

	template <typename OtherType>
	TSharedPtr(TSharedPtr<OtherType, Mode> const& otherSharedPtr, ObjectType* inObject)
		: object(inObject)
		, sharedReferenceCount(otherSharedPtr.sharedReferenceCount)
	{

	}

	template <typename OtherType>
	TSharedPtr(TSharedPtr<OtherType, Mode>&& otherSharedPtr, ObjectType* inObject)
		: object(inObject)
		, sharedReferenceCount(MoveTemp(otherSharedPtr.sharedReferenceCount))
	{
		otherSharedPtr.object = nullptr;
	}

	template <typename OtherType>
	TSharedPtr(TSharedRef<OtherType, Mode> const& otherSharedRef, ObjectType* inObject)
		: object(inObject)
		, sharedReferenceCount(otherSharedRef.sharedReferenceCount)
	{

	}

	FORCE_INLINE TSharedPtr& operator=(Fly3DPrivateSharedPointer::FNullTag* tag)
	{
		Reset();
		return *this;
	}

	FORCE_INLINE TSharedPtr& operator=(TSharedPtr const& inSharedPtr)
	{
		TSharedPtr temp = inSharedPtr;
		Swap(temp, *this);
		return *this;
	}

	FORCE_INLINE TSharedPtr& operator=(TSharedPtr&& inSharedPtr)
	{
		if (this != &inSharedPtr)
		{
			object = inSharedPtr.object;
			inSharedPtr.object = nullptr;
			sharedReferenceCount = MoveTemp(inSharedPtr.sharedReferenceCount);
		}
		return *this;
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	FORCE_INLINE TSharedPtr& operator=(Fly3DPrivateSharedPointer::FRawPtrProxy<OtherType> const& inRawPtrProxy)
	{
		*this = TSharedPtr<ObjectType, Mode>(inRawPtrProxy);
		return *this;
	}

	FORCE_INLINE TSharedRef<ObjectType, Mode> ToSharedRef() const
	{
		return TSharedRef<ObjectType, Mode>(*this);
	}

	FORCE_INLINE ObjectType* Get() const
	{
		return object;
	}

	FORCE_INLINE explicit operator bool() const
	{
		return object != nullptr;
	}

	FORCE_INLINE const bool IsValid() const
	{
		return object != nullptr;
	}

	FORCE_INLINE typename FMakeReferenceTo<ObjectType>::Type operator*() const
	{
		return *object;
	}

	FORCE_INLINE ObjectType* operator->() const
	{
		return object;
	}

	FORCE_INLINE void Reset()
	{
		*this = TSharedPtr<ObjectType, Mode>();
	}

	FORCE_INLINE const int32 GetSharedReferenceCount() const
	{
		return sharedReferenceCount.GetSharedReferenceCount();
	}

	FORCE_INLINE const bool IsUnique() const
	{
		return sharedReferenceCount.IsUnique();
	}

private:

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	explicit TSharedPtr(TWeakPtr<OtherType, Mode> const& inWeakPtr)
		: object(nullptr)
		, sharedReferenceCount(inWeakPtr.weakReferenceCount)
	{
		if (sharedReferenceCount.IsValid())
		{
			object = inWeakPtr.object;
		}
	}

	template<class OtherType, ESPMode OtherMode> 
	friend class TSharedPtr;

	template<class OtherType, ESPMode OtherMode> 
	friend class TSharedRef;

	template<class OtherType, ESPMode OtherMode> 
	friend class TWeakPtr;

	template<class OtherType, ESPMode OtherMode> 
	friend class TSharedFromThis;

private:

	ObjectType* object;

	Fly3DPrivateSharedPointer::FSharedReferencer<Mode> sharedReferenceCount;
};

template<class ObjectType, ESPMode Mode> 
struct TIsZeroConstructType<TSharedPtr<ObjectType, Mode>> 
{ 
	enum 
	{ 
		Value = true 
	};
};

template<class ObjectType, ESPMode Mode>
class TWeakPtr
{
public:

	using ElementType = ObjectType;

	TWeakPtr(Fly3DPrivateSharedPointer::FNullTag* tag = nullptr)
		: object(nullptr)
		, weakReferenceCount()
	{ 

	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TWeakPtr(TSharedRef<OtherType, Mode> const& inSharedRef)
		: object(inSharedRef.object)
		, weakReferenceCount(inSharedRef.sharedReferenceCount)
	{ 

	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TWeakPtr(TSharedPtr<OtherType, Mode> const& inSharedPtr)
		: object(inSharedPtr.object)
		, weakReferenceCount(inSharedPtr.sharedReferenceCount)
	{

	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TWeakPtr(TWeakPtr<OtherType, Mode> const& inWeakPtr)
		: object(inWeakPtr.object)
		, weakReferenceCount(inWeakPtr.weakReferenceCount)
	{ 

	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	TWeakPtr(TWeakPtr<OtherType, Mode>&& inWeakPtr)
		: object(inWeakPtr.object)
		, weakReferenceCount(MoveTemp(inWeakPtr.weakReferenceCount))
	{
		inWeakPtr.object = nullptr;
	}

	TWeakPtr(TWeakPtr const& inWeakPtr)
		: object(inWeakPtr.object)
		, weakReferenceCount(inWeakPtr.weakReferenceCount)
	{

	}

	TWeakPtr(TWeakPtr&& inWeakPtr)
		: object(inWeakPtr.object)
		, weakReferenceCount(MoveTemp(inWeakPtr.weakReferenceCount))
	{
		inWeakPtr.object = nullptr;
	}

	FORCE_INLINE TWeakPtr& operator=(Fly3DPrivateSharedPointer::FNullTag* tag)
	{
		Reset();
		return *this;
	}

	FORCE_INLINE TWeakPtr& operator=(TWeakPtr const& inWeakPtr)
	{
		TWeakPtr temp = inWeakPtr;
		Swap(temp, *this);
		return *this;
	}

	FORCE_INLINE TWeakPtr& operator=(TWeakPtr&& inWeakPtr)
	{
		if (this != &inWeakPtr)
		{
			object = inWeakPtr.object;
			inWeakPtr.object = nullptr;
			weakReferenceCount = MoveTemp(inWeakPtr.weakReferenceCount);
		}
		return *this;
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	FORCE_INLINE TWeakPtr& operator=(TWeakPtr<OtherType, Mode> const& inWeakPtr)
	{
		object = inWeakPtr.Pin().Get();
		weakReferenceCount = inWeakPtr.weakReferenceCount;
		return *this;
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	FORCE_INLINE TWeakPtr& operator=(TWeakPtr<OtherType, Mode>&& inWeakPtr)
	{
		object = inWeakPtr.object;
		inWeakPtr.object = nullptr;
		weakReferenceCount = MoveTemp(inWeakPtr.weakReferenceCount);
		return *this;
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	FORCE_INLINE TWeakPtr& operator=(TSharedRef<OtherType, Mode> const& inSharedRef)
	{
		object = inSharedRef.object;
		weakReferenceCount = inSharedRef.sharedReferenceCount;
		return *this;
	}

	template <
		typename OtherType,
		typename = typename TEnableIf<TPointerIsConvertibleFromTo<OtherType, ObjectType>::Value>::Type
	>
	FORCE_INLINE TWeakPtr& operator=(TSharedPtr<OtherType, Mode> const& inSharedPtr)
	{
		object = inSharedPtr.object;
		weakReferenceCount = inSharedPtr.sharedReferenceCount;
		return *this;
	}

	FORCE_INLINE TSharedPtr<ObjectType, Mode> Pin() const
	{
		return TSharedPtr<ObjectType, Mode>(*this);
	}

	FORCE_INLINE const bool IsValid() const
	{
		return object != nullptr && weakReferenceCount.IsValid();
	}

	FORCE_INLINE void Reset()
	{
		*this = TWeakPtr<ObjectType, Mode>();
	}

	FORCE_INLINE bool HasSameObject(const void* inOtherPtr) const
	{
		return Pin().Get() == inOtherPtr;
	}

private:

	template<class OtherType, ESPMode OtherMode> 
	friend class TWeakPtr;

	template<class OtherType, ESPMode OtherMode> 
	friend class TSharedPtr;

private:

	ObjectType* object;

	Fly3DPrivateSharedPointer::FWeakReferencer<Mode> weakReferenceCount;
};

template<class T, ESPMode Mode> 
struct TIsWeakPointerType<TWeakPtr<T, Mode>> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template<class T, ESPMode Mode> 
struct TIsZeroConstructType<TWeakPtr<T, Mode>> 
{ 
	enum 
	{ 
		Value = true 
	}; 
};

template<class ObjectType, ESPMode Mode>
class TSharedFromThis
{
public:

	TSharedRef<ObjectType, Mode> AsShared()
	{
		TSharedPtr<ObjectType, Mode> sharedThis(weakThis.Pin());
		return sharedThis.ToSharedRef();
	}

	TSharedRef<ObjectType const, Mode> AsShared() const
	{
		TSharedPtr<ObjectType const, Mode> sharedThis(weakThis);
		return sharedThis.ToSharedRef();
	}

protected:

	template<class OtherType>
	FORCE_INLINE static TSharedRef<OtherType, Mode> SharedThis(OtherType* thisPtr)
	{
		return StaticCastSharedRef<OtherType>(thisPtr->AsShared());
	}

	template<class OtherType>
	FORCE_INLINE static TSharedRef<OtherType const, Mode> SharedThis(const OtherType* thisPtr)
	{
		return StaticCastSharedRef<OtherType const>(thisPtr->AsShared());
	}

public:

	template<class SharedPtrType, class OtherType>
	FORCE_INLINE void UpdateWeakReferenceInternal(TSharedPtr<SharedPtrType, Mode> const* inSharedPtr, OtherType* inObject) const
	{
		if (!weakThis.IsValid())
		{
			weakThis = TSharedPtr<ObjectType, Mode>(*inSharedPtr, inObject);
		}
	}

	template<class SharedRefType, class OtherType>
	FORCE_INLINE void UpdateWeakReferenceInternal(TSharedRef<SharedRefType, Mode> const* inSharedRef, OtherType* inObject) const
	{
		if (!weakThis.IsValid() )
		{
			weakThis = TSharedRef<ObjectType, Mode>(*inSharedRef, inObject);
		}
	}

	FORCE_INLINE bool DoesSharedInstanceExist() const
	{
		return weakThis.IsValid();
	}

protected:

	TSharedFromThis() 
	{ 

	}

	TSharedFromThis(TSharedFromThis const&) 
	{ 

	}

	FORCE_INLINE TSharedFromThis& operator=(TSharedFromThis const&)
	{
		return *this;
	}

	~TSharedFromThis() 
	{ 

	}

private:

	mutable TWeakPtr<ObjectType, Mode> weakThis;	
};

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedRef<ObjectTypeA, Mode> const& inSharedRefA, TSharedRef<ObjectTypeB, Mode> const& inSharedRefB)
{
	return &(inSharedRefA.Get()) == &(inSharedRefB.Get());
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedRef<ObjectTypeA, Mode> const& inSharedRefA, TSharedRef<ObjectTypeB, Mode> const& inSharedRefB)
{
	return &(inSharedRefA.Get()) != &(inSharedRefB.Get());
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedPtr<ObjectTypeA, Mode> const& inSharedPtrA, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtrB)
{
	return inSharedPtrA.Get() == inSharedPtrB.Get();
}

template<class ObjectTypeA, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedPtr<ObjectTypeA, Mode> const& inSharedPtrA, TYPE_OF_NULLPTR)
{
	return !inSharedPtrA.IsValid();
}

template<class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TYPE_OF_NULLPTR, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtrB)
{
	return !inSharedPtrB.IsValid();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedPtr<ObjectTypeA, Mode> const& inSharedPtrA, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtrB)
{
	return inSharedPtrA.Get() != inSharedPtrB.Get();
}

template<class ObjectTypeA, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedPtr<ObjectTypeA, Mode> const& inSharedPtrA, TYPE_OF_NULLPTR)
{
	return inSharedPtrA.IsValid();
}

template<class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TYPE_OF_NULLPTR, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtrB)
{
	return inSharedPtrB.IsValid();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedRef<ObjectTypeA, Mode> const& inSharedRef, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtr)
{
	return inSharedPtr.IsValid() && inSharedPtr.Get() == &(inSharedRef.Get());
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedRef<ObjectTypeA, Mode> const& inSharedRef, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtr)
{
	return !inSharedPtr.IsValid() || (inSharedPtr.Get() != &(inSharedRef.Get()));
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedPtr<ObjectTypeB, Mode> const& inSharedPtr, TSharedRef<ObjectTypeA, Mode> const& inSharedRef)
{
	return inSharedRef == inSharedPtr;
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedPtr<ObjectTypeB, Mode> const& inSharedPtr, TSharedRef<ObjectTypeA, Mode> const& inSharedRef)
{
	return inSharedRef != inSharedPtr;
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return inWeakPtrA.Pin().Get() == inWeakPtrB.Pin().Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TSharedRef<ObjectTypeB, Mode> const& inSharedRefB)
{
	return inWeakPtrA.Pin().Get() == &inSharedRefB.Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtrB)
{
	return inWeakPtrA.Pin().Get() == inSharedPtrB.Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedRef<ObjectTypeA, Mode> const& inSharedRefA, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return &inSharedRefA.Get() == inWeakPtrB.Pin().Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TSharedPtr<ObjectTypeA, Mode> const& inSharedPtrA, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return inSharedPtrA.Get() == inWeakPtrB.Pin().Get();
}

template<class ObjectTypeA, ESPMode Mode>
FORCE_INLINE bool operator==(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TYPE_OF_NULLPTR)
{
	return !inWeakPtrA.IsValid();
}

template<class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator==(TYPE_OF_NULLPTR, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return !inWeakPtrB.IsValid();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return inWeakPtrA.Pin().Get() != inWeakPtrB.Pin().Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TSharedRef<ObjectTypeB, Mode> const& inSharedRefB)
{
	return inWeakPtrA.Pin().Get() != &inSharedRefB.Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TSharedPtr<ObjectTypeB, Mode> const& inSharedPtrB)
{
	return inWeakPtrA.Pin().Get() != inSharedPtrB.Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedRef<ObjectTypeA, Mode> const& inSharedRefA, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return &inSharedRefA.Get() != inWeakPtrB.Pin().Get();
}

template<class ObjectTypeA, class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TSharedPtr<ObjectTypeA, Mode> const& inSharedPtrA, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return inSharedPtrA.Get() != inWeakPtrB.Pin().Get();
}

template<class ObjectTypeA, ESPMode Mode>
FORCE_INLINE bool operator!=(TWeakPtr<ObjectTypeA, Mode> const& inWeakPtrA, TYPE_OF_NULLPTR)
{
	return inWeakPtrA.IsValid();
}

template<class ObjectTypeB, ESPMode Mode>
FORCE_INLINE bool operator!=(TYPE_OF_NULLPTR, TWeakPtr<ObjectTypeB, Mode> const& inWeakPtrB)
{
	return inWeakPtrB.IsValid();
}

template<class CastToType, class CastFromType, ESPMode Mode>
FORCE_INLINE TSharedPtr<CastToType, Mode> StaticCastSharedPtr(TSharedPtr<CastFromType, Mode> const& inSharedPtr)
{
	return TSharedPtr<CastToType, Mode>(inSharedPtr, Fly3DPrivateSharedPointer::FStaticCastTag());
}

template<class CastToType, class CastFromType, ESPMode Mode>
FORCE_INLINE TSharedRef<CastToType, Mode> ConstCastSharedRef(TSharedRef<CastFromType, Mode> const& inSharedRef)
{
	return TSharedRef<CastToType, Mode>(inSharedRef, Fly3DPrivateSharedPointer::FConstCastTag());
}

template<class CastToType, class CastFromType, ESPMode Mode>
FORCE_INLINE TSharedPtr<CastToType, Mode> ConstCastSharedPtr( TSharedPtr<CastFromType, Mode> const& inSharedPtr)
{
	return TSharedPtr<CastToType, Mode>(inSharedPtr, Fly3DPrivateSharedPointer::FConstCastTag());
}

template<class ObjectType>
FORCE_INLINE Fly3DPrivateSharedPointer::FRawPtrProxy<ObjectType> MakeShareable(ObjectType* inObject)
{
	return Fly3DPrivateSharedPointer::FRawPtrProxy<ObjectType>(inObject);
}

template<class ObjectType, class DeleterType>
FORCE_INLINE Fly3DPrivateSharedPointer::FRawPtrProxy<ObjectType> MakeShareable(ObjectType* inObject, DeleterType&& inDeleter)
{
	return Fly3DPrivateSharedPointer::FRawPtrProxy<ObjectType>(inObject, Forward<DeleterType>(inDeleter));
}

template <typename InObjectType, ESPMode InMode = ESPMode::ThreadUnSafe, typename... InArgTypes>
FORCE_INLINE TSharedRef<InObjectType, InMode> MakeShared(InArgTypes&&... args)
{
	Fly3DPrivateSharedPointer::TIntrusiveReferenceController<InObjectType>* controller = Fly3DPrivateSharedPointer::NewIntrusiveReferenceController<InObjectType>(Forward<InArgTypes>(args)...);
	return Fly3DPrivateSharedPointer::MakeSharedRef<InObjectType, InMode>(controller->GetObjectPtr(), (Fly3DPrivateSharedPointer::FReferenceControllerBase*)controller);
}