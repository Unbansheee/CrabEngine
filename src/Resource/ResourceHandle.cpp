#include "ResourceHandle.h"
#include "Resource.h"

ResourceRef::ResourceRef() : typeHash(typeid(Resource))
{
}

StrongResourceRef::StrongResourceRef(std::type_index type) : typeHash(type)
{
}

StrongResourceRef::StrongResourceRef() : typeHash(typeid(Resource))
{
}

/*
bool ResourceHandle::IsValid() const
{
    return ResourceDB::Get().IsResourceIDValid(ResourceID);
}
*/
