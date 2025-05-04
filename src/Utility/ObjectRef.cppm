module;

#pragma once
#include "rocket/rocket.hpp"

export module Engine.Object.Ref;
import Engine.Object.ObservableDtor;
import Engine.Assert;
export class Object;

export class _ObjectRefBase {

};

export template <typename T = Object>
class ObjectRef;

template <typename T>
struct ObjectRefUnderlyingType {
    using type = T; // Default to T if it’s not an ObjectRef
};

template <typename U>
struct ObjectRefUnderlyingType<ObjectRef<U>> {
    using type = U; // Extract U from ObjectRef<U>
};

// An ObjectRef is a wrapper for an Object pointer. Upon destruction of the underlying object, the pointer will be set to null
// Objects referenced by a Handle must inherit observable_dtor
// The purpose of this class is to have a weak_ptr alternative that does require the ownership semantics of shared_ptr
// Holding a handle to an object will not keep it alive

export template <typename T>
class ObjectRef : rocket::trackable, public _ObjectRefBase
{
public:
    using element_type = T;

    // Empty constructor
    ObjectRef();
    // Dtor
    ~ObjectRef();

    // Ptr constructor
    template <typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
    explicit ObjectRef(U* ptr);

    // Move constructor
    template <typename U, typename = std::enable_if_t<std::is_base_of_v<T, U>>>
    ObjectRef(ObjectRef<U>&& other);

    // Copy constructor
    ObjectRef(const ObjectRef& other) {
        Clear();
        if (other.IsValid()) {
            connection = other.node->destroyed_signal.template connect<&ObjectRef::OnResourceDestroyed>(this);
            node = other.node;
        } else {
            node = nullptr;
        }
    }

    // IsValid check
    explicit operator bool() const;
    operator ObjectRef<Object>() const;
    // Dereference to pointer
    T* operator->() const;
    // Underlying object comparison
    bool operator==(const ObjectRef& other) const;
    // Underlying object comparison
    bool operator==(const T*& other) const;
    // Underlying object comparison
    bool operator==(T* other) const;
    // Underlying object comparison
    bool operator==(const T* other) const;

    // Assign from another ObjectRef
    ObjectRef<T>& operator=(const ObjectRef<T>& other);

    // Pointer assignment
    ObjectRef<T>& operator=(T* other);

    // Move assignment
    template<typename U>
    ObjectRef<T>& operator=(ObjectRef<U>&& other) {
        if (this != &other) {
            Clear();
            if (other.IsValid()) {
                node = static_cast<T*>(other.node);
                connection = std::move(other.connection);
                other.node = nullptr;
                other.connection.disconnect();
            }
        }
        return *this;
    }

    template <typename Target>
    ObjectRef<typename ObjectRefUnderlyingType<Target>::type> StaticCast() const;

    template <typename Target>
    ObjectRef<typename ObjectRefUnderlyingType<Target>::type> Cast() const;

    // Does the underlying object still exist
    bool IsValid() const;

    // Sets the underlying object to a new one
    void Assign(T* obj);
    void AssignUnsafe(Object* obj);
    T* Get() const;
    void Clear();

private:
    T* node;
    typename rocket::connection connection;

    void OnResourceDestroyed();
};

template <typename T>
ObjectRef<T>::~ObjectRef()
{
    Clear();
}

template<typename T>
template<typename U, typename>
ObjectRef<T>::ObjectRef(U *ptr) {
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");

    Clear();
    if (ptr != nullptr) {
        connection = ptr->destroyed_signal.template connect<&ObjectRef::OnResourceDestroyed>(this);
        node = static_cast<T*>(ptr);
    }
}

template<typename T>
template<typename U, typename>
ObjectRef<T>::ObjectRef(ObjectRef<U> &&other) {
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
    Clear();
    if (other.IsValid()) {
        assert(U::GetStaticClass().IsSubclassOf(T::GetStaticClass()));
    }

    // Transfer resources from 'other'
    node = static_cast<T*>(other.node);
    connection = std::move(other.connection);

    // Clear 'other'
    other.Clear();
}

template <typename T>
ObjectRef<T>::operator bool() const
{
    return IsValid();
}

template<typename T>
ObjectRef<T>::operator ObjectRef<Object>() const {
    return ObjectRef<Object>(node);
}

template <typename T>
T* ObjectRef<T>::operator->() const
{
    return node;
}

template <typename T>
bool ObjectRef<T>::operator==(const ObjectRef& other) const
{
    return node == other.node;
}

template <typename T>
bool ObjectRef<T>::operator==(const T*& other) const
{
    return node == other;
}

template <typename T>
bool ObjectRef<T>::operator==(T* other) const
{
    return node == other;
}

template <typename T>
bool ObjectRef<T>::operator==(const T* other) const
{
    return node == other;
}

template <typename T>
ObjectRef<T>& ObjectRef<T>::operator=(T* other)
{
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");

    if (node == other) return *this;



    Clear();
    if (other != nullptr) {
        connection = other->destroyed_signal.template connect<&ObjectRef::OnResourceDestroyed>(this);
        node = other;
    }
    return *this;
}


template<typename T>
ObjectRef<T> & ObjectRef<T>::operator=(const ObjectRef &other) {
    Clear();

    if (other.IsValid()) {
        connection = other.Get()->destroyed_signal.template connect<&ObjectRef::OnResourceDestroyed>(this);
        node = other.Get();
    }

    return (*this);
}

template <typename T>
ObjectRef<T>::ObjectRef(): node(nullptr)
{
    Clear();
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
}

template<typename T>
template<typename Target>
ObjectRef<typename ObjectRefUnderlyingType<Target>::type> ObjectRef<T>::StaticCast() const {
    using U = typename ObjectRefUnderlyingType<Target>::type;

    static_assert(std::is_base_of_v<T, U>, "U must be derived from T");
    return ObjectRef<U>(static_cast<U*>(node));
}

template<typename T>
template<typename Target>
ObjectRef<typename ObjectRefUnderlyingType<Target>::type> ObjectRef<T>::Cast() const {
    using U = typename ObjectRefUnderlyingType<Target>::type;

    if constexpr(std::is_base_of_v<_ObjectRefBase, U>) {
        if (U::Type::GetStaticClass().IsSubclassOf(T::GetStaticClass())) {
            return ObjectRef<typename U::Type>(static_cast<typename U::Type*>(node));
        }
    }
    else {
        if (U::GetStaticClass().IsSubclassOf(T::GetStaticClass())) {
            return ObjectRef<U>(static_cast<U*>(node));
        }
    }

    return ObjectRef<U>();

}


template <typename T>
bool ObjectRef<T>::IsValid() const
{
    return node != nullptr && connection.is_connected();
    //return node != nullptr && connection.is_connected();
}

template <typename T>
void ObjectRef<T>::Assign(T* obj)
{
    Clear();

    /*
    if (baseClass) {
        assert(T::GetStaticClass().IsSubclassOf(*baseClass));
    }
    */


    node = obj;
    if (node != nullptr)
    {
        node->destroyed_signal.connect<&ObjectRef::OnResourceDestroyed>(this);
    }
}

template<typename T>
void ObjectRef<T>::AssignUnsafe(Object *obj) {
    Clear();
    node = static_cast<T*>(obj);
    assert(node != nullptr);
    if (node != nullptr)
    {
        node->destroyed_signal.connect<&ObjectRef::OnResourceDestroyed>(this);
    }
}

template <typename T>
T* ObjectRef<T>::Get() const
{
    return node;
}

template <typename T>
void ObjectRef<T>::Clear()
{
    node = nullptr;
    if (connection)
    {
        connection.disconnect();
    }
    connection = {};

}

template <typename T>
void ObjectRef<T>::OnResourceDestroyed()
{
    Clear();
    this->node = nullptr;
}
