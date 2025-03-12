#pragma once
#include <typeindex>
#include "CrabTypes.h"
#include "ResourceDB.h"
#include "Core/UID.h"

struct ResourceHandle
{
    ResourceHandle() {}
    ResourceHandle(UID id, std::type_index type) : ResourceID(id), typeID(type){}
    ~ResourceHandle() {};
    
    UID ResourceID = UID::empty();
    std::type_index typeID = typeid(bool);

public:
    template<typename T>
    SharedRef<T> Resolve() const;

    bool IsValid() const;
};

template <typename T>
SharedRef<T> ResourceHandle::Resolve() const
{
    auto res = ResourceDB::Get().GetResource(ResourceID);
    assert(res);

    auto casted = std::dynamic_pointer_cast<T>(res);
    assert(casted);

    return casted;
}