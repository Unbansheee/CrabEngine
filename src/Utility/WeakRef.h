#pragma once

#pragma once
#include <iostream>
#include <ostream>

#include "Nodes/Node.h"
#include "ObservableDtor.h"
#include "rocket/rocket.hpp"

// A Handle is a wrapper for a pointer. Upon destruction of the underlying object, the pointer will be set to null
// Objects referenced by a Handle must inherit observable_dtor
// The purpose of this class is to have a weak_ptr alternative that does require the ownership semantics of shared_ptr
// Holding a handle to an object will not keep it alive
template <typename T>
class WeakRef : rocket::trackable
{
public:
    // Empty constructor
    WeakRef();
    // Construct a handle from another object
    WeakRef(T* node);
    // Construct a handle from another handle
    WeakRef(const WeakRef& other);
    // Dtor
    ~WeakRef();

    // IsValid check
    operator bool() const;
    // Dereference to pointer
    T* operator->() const;
    // Underlying object comparison
    bool operator==(const WeakRef& other) const;
    // Underlying object comparison
    bool operator==(const T*& other) const;
    // Underlying object comparison
    bool operator==(T* other) const;
    // Underlying object comparison
    bool operator==(const T* other) const;
    // Pointer assignment
    WeakRef& operator=(T* other);
    // Pointer assignment
    WeakRef& operator=(const T*& other);

    // Does the underlying object still exist
    bool IsValid() const;

    // Sets the underlying object to a new one
    void Assign(T* obj);
    T* Get() const;
    void Clear();

private:
    T* node;
    typename rocket::connection connection;

    void OnResourceDestroyed();
};

template <typename T>
WeakRef<T>::WeakRef(T* node)
{
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
    if (!node) return;
    this->node = node;
    node->destroyed_signal += [&]()
    {
        connection = node->destroyed_signal.template connect<&WeakRef::OnResourceDestroyed>(this);
    };
}

template <typename T>
WeakRef<T>::WeakRef(const WeakRef& other)
{
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
    if (other.IsValid())
    {
        node = other.node;
        connection = node->destroyed_signal.template connect<&WeakRef::OnResourceDestroyed>(this);
    }
}

template <typename T>
WeakRef<T>::~WeakRef()
{
    Clear();
}

template <typename T>
WeakRef<T>::operator bool() const
{
    return IsValid();
}

template <typename T>
T* WeakRef<T>::operator->() const
{
    return node;
}

template <typename T>
bool WeakRef<T>::operator==(const WeakRef& other) const
{
    return node == other.node;
}

template <typename T>
bool WeakRef<T>::operator==(const T*& other) const
{
    return node == other;
}

template <typename T>
bool WeakRef<T>::operator==(T* other) const
{
    return node == other;
}

template <typename T>
bool WeakRef<T>::operator==(const T* other) const
{
    return node == other;
}

template <typename T>
WeakRef<T>& WeakRef<T>::operator=(T* other)
{
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
    Clear();
    connection = other->destroyed_signal.template connect<&WeakRef::OnResourceDestroyed>(this);
    node = other;
    return *this;
}

template <typename T>
WeakRef<T>& WeakRef<T>::operator=(const T*& other)
{
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
    Clear();
    connection = other->destroyed_signal.template connect<&WeakRef::OnResourceDestroyed>(this);
    node = other;
    return *this;
}

template <typename T>
WeakRef<T>::WeakRef(): node(nullptr)
{
    static_assert(std::is_base_of_v<observable_dtor, T>, "T must implement observable_dtor");
}

template <typename T>
bool WeakRef<T>::IsValid() const
{
    return node != nullptr;
}

template <typename T>
void WeakRef<T>::Assign(T* obj)
{
    Clear();

    node = obj;
    if (node != nullptr)
    {
        node->destroyed_signal.connect<&WeakRef::OnResourceDestroyed>(this);
    }
}

template <typename T>
T* WeakRef<T>::Get() const
{
    return node;
}

template <typename T>
void WeakRef<T>::Clear()
{
    node = nullptr;
    if (connection)
    {
        connection.disconnect();
    }
}

template <typename T>
void WeakRef<T>::OnResourceDestroyed()
{
    this->node = nullptr;
}
