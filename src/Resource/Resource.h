#pragma once
#include <filesystem>

#include "Core/ClassDB.h"
#include "Core/Object.h"
#include "Utility/ObservableDtor.h"


class Resource : public Object, public observable_dtor
{
    friend class ResourceManager;
    
public:
    CRAB_CLASS(Resource, Object)
    CLASS_FLAG(EditorVisible)
    
    BEGIN_PROPERTIES
        ADD_PROPERTY("ResourceFilePath", resourceFilePath)
    END_PROPERTIES
    
    virtual void Serialize(nlohmann::json& archive) override { Object::Serialize(archive); }
    virtual void Deserialize(nlohmann::json& archive) override { Object::Deserialize(archive); }
    virtual void LoadData() {};  // For explicit loading
    virtual bool IsLoaded() const {return loaded;}

    const std::string& GetResourcePath() const { return resourceFilePath; }

protected:
    std::string resourceFilePath;
    std::atomic<bool> loaded{false};
    
    /*
    template<typename T>
    static std::shared_ptr<T> CreateResource()
    {
        //static_assert(std::is_base_of_v<T, Resource>, "Only Resource types can be created with CreateResource");
        auto res = MakeShared<T>();
        res->typeID = std::type_index<T>(typeid(T));
        ResourceDB::Get().RegisterResource(res);
        return res;
    }

    template<typename T>
    static std::shared_ptr<T> CreateResource(const std::filesystem::path& file)
        {
            //static_assert(std::is_base_of_v<T, Resource>, "Only Resource types can be created with CreateResource");
            auto res = MakeShared<T>();
            res->typeID = std::type_index(typeid(T));
            res->InitializeFromFile(file);
            ResourceDB::Get().RegisterResource(res);
            return res;
        }

    virtual void InitializeFromFile(const std::filesystem::path& file){};
    
    virtual ~Resource() override
    {
        ResourceDB::Get().UnregisterResource(GetID());
    }
    
    Resource (const Resource&) = delete;
    Resource& operator= (const Resource&) = delete;

    ResourceHandle Handle()
    {
        return ResourceHandle{
            GetID(),
            typeID
        };
    }
    */
protected:
    //Resource() = default;
private:
    //std::type_index typeID = std::type_index(typeid(Resource));
};

