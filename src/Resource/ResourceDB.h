#pragma once
#include <memory>
#include "Core/UID.h"

class ResourceDB
{
    friend class Resource;
public:
    static ResourceDB& Get()
    {
        static ResourceDB s;
        return s;
    }

    bool IsResourceIDValid(UID id);
    std::shared_ptr<Resource> GetResource(UID id);
    
    void RegisterResource(const std::shared_ptr<Resource>& resource);
    void UnregisterResource(UID id);
    
private:
    std::unordered_map<UID, std::weak_ptr<Resource>> ResourceCache;    
};
