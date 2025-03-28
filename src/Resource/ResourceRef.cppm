module;

#pragma once

#include <memory>
#include <stdexcept>
#include <string>
#include <typeindex>

export module Engine.Resource.Ref;
//import class_type;
//import resource;
//import reflection;

export class Resource;
export struct ClassType;

export class ResourceRef {
public:
    template<typename T>
    ResourceRef(const std::shared_ptr<T>& res) : resource(res), typeHash(typeid(T))
    {
        if(res) resourcePath = res->GetSourcePath();
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

/*
class StrongResourceRef {
public:
    // Constructor with explicit type constraint
    template<IsResource T>
    StrongResourceHandle(std::shared_ptr<T> resource)
        : m_resource(resource),
          m_classType(T::StaticClass()) // From reflection system
    {
        static_assert(std::is_base_of_v<Resource, T>, 
            "ResourceHandle can only manage Resource-derived types");
    }

    StrongResourceRef(const ClassType& Filter) : typeFilter(Filter)
    {
    }
    
    ~StrongResourceRef() {};

    template<typename T>
    std::shared_ptr<T> Get() const
    {
        return std::dynamic_pointer_cast<T>(resource);
    }

    operator bool() const { return resource != nullptr; }

    const std::string& GetResourcePath() { return resourcePath; }
    bool IsCompatible(const std::shared_ptr<Resource>& with);
    
private:
    std::shared_ptr<Resource> resource = nullptr; // Maintains ownership
    const ClassType& typeFilter = Resource::GetStaticClass();
    std::string resourcePath{};
};
*/

export class StrongResourceRef {
public:
    // Default constructor for empty handle
    StrongResourceRef() = default;

    // Main constructor with type constraint
    StrongResourceRef(std::shared_ptr<Resource> resource, const ClassType* filter);

    // Type-safe assignment using constructor's constraints
    StrongResourceRef& operator=(std::shared_ptr<Resource> resource);

    // Get resource with type checking
    template <typename T>
    std::shared_ptr<T> Get() const {
        if (!m_resource) return nullptr;
        if (!IsType(T::GetStaticClass())) {
            throw std::runtime_error("Resource type mismatch in Get()");
        }
        return std::static_pointer_cast<T>(m_resource);
    }

    // Runtime type checking
    bool IsType(const ClassType& type) const;
    
    // For std::variant compatibility
    bool operator==(const StrongResourceRef& other) const {
        return m_resource == other.m_resource;
    }

    operator bool() const
    {
        return m_resource.get();
    }

    // Set explicit type filter (for editor properties)
    void SetTypeFilter(const ClassType& filterType);
    
    // Get current type filter
    const ClassType* GetTypeFilter() const { return m_classType; }

    bool IsResourceCompatible(const std::shared_ptr<Resource>& resource) const;


private:
    std::shared_ptr<Resource> m_resource;
    const ClassType* m_classType = nullptr;

};

