#include "ResourceDB.h"

#include "Resource.h"

bool ResourceDB::IsResourceIDValid(UID id)
{
    return ResourceCache.contains(id) && !ResourceCache[id].expired();
}

std::shared_ptr<Resource> ResourceDB::GetResource(UID id)
{
    if (ResourceCache.contains(id))
    {
        auto res = ResourceCache[id];
        if (auto shared = res.lock())
        {
            return shared;
        }
    }
    return nullptr;
}

void ResourceDB::RegisterResource(const std::shared_ptr<Resource>& resource)
{
    ResourceCache[resource->GetID()] = resource;    
}

void ResourceDB::UnregisterResource(UID id)
{
    ResourceCache.erase(id);
}
