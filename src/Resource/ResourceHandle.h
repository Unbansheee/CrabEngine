#pragma once
#include <memory>
#include <string>
#include <typeindex>

class Resource;

class ResourceRef {
public:
    template<typename T>
    ResourceRef(const std::shared_ptr<T>& res) : resource(res), typeHash(typeid(T))
    {
        if(res) resourcePath = res->GetResourcePath();
    }

    ResourceRef(std::type_index type) : typeHash(type) {};
    ResourceRef();
    ~ResourceRef() {};


    template<typename T>
    std::shared_ptr<T> Get() const
    {
        auto safe = resource.lock();
        if (!safe) return nullptr;
        
        return std::dynamic_pointer_cast<T>(safe);
    }
    operator bool() const { return !resource.expired(); }
    
    std::type_index GetTypeHash() const { return typeHash; }
    std::string GetResourcePath() const { return resourcePath; }
    
private:
    std::weak_ptr<Resource> resource = {};
    std::type_index typeHash;
    std::string resourcePath{};
};

class StrongResourceRef {
public:
    template<typename T>
    StrongResourceRef(const std::shared_ptr<T>& res)
        : resource(res), typeHash(typeid(T))
    {
        if(res) resourcePath = res->GetResourcePath();
    }

    StrongResourceRef(std::type_index type);
    StrongResourceRef();
    
    ~StrongResourceRef() {};

    template<typename T>
    std::shared_ptr<T> Get() const
    {
        return std::dynamic_pointer_cast<T>(resource);
    }
    std::type_index GetTypeHash() const { return typeHash; }
    operator bool() const { return resource != nullptr; }

    const std::string& GetResourcePath() {return resourcePath;};
    
private:
    std::shared_ptr<Resource> resource = nullptr; // Maintains ownership
    std::type_index typeHash;
    std::string resourcePath{};
};