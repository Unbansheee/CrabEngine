module;

#pragma once

#include <filesystem>
#include "ReflectionMacros.h"
//#include "Importers/ResourceImporter.cppm"
//#include "Utility/ObservableDtor.h"

export module resource;
export import object;
export import observable_dtor;
import resource_importer;
import import_settings;
import class_type;

export class ImportManager;

export class Resource : public Object, public observable_dtor
{
    friend class ResourceManager;
    friend class ImportManager;
    
public:
    CRAB_CLASS(Resource, Object)
    CLASS_FLAG(EditorVisible)
    
    BEGIN_PROPERTIES
        ADD_PROPERTY("Name", name)
        ADD_PROPERTY_FLAGS("ResourceFilePath", resourceFilePath, Property::Flags::HideFromInspector)
        ADD_PROPERTY_FLAGS("SourcePath", sourcePath, Property::Flags::HideFromInspector)
        ADD_PROPERTY_FLAGS("IsSourceImported", bIsSourceImported, Property::Flags::HideFromInspector)
    END_PROPERTIES

    using Ptr = std::shared_ptr<Resource>;
    
    virtual void Serialize(nlohmann::json& archive) override { Object::Serialize(archive); }
    virtual void Deserialize(nlohmann::json& archive) override { Object::Deserialize(archive); }
    virtual void LoadData() { loaded = true; };  // For explicit loading
    virtual bool IsLoaded() const {return loaded;}

    const std::string& GetName() const {return name;}
    const std::string& GetResourcePath() const { return resourceFilePath; }
    bool IsSourceImported() const { return bIsSourceImported; }
    const std::string& GetSourcePath() {return sourcePath; }
    const std::shared_ptr<ImportSettings>& GetImportSettings() { return importSettings; };
    bool IsInline() const { return bIsInline; }
protected:
    std::string name;
    bool bIsSourceImported;
    bool bIsInline = true;
    std::shared_ptr<ImportSettings> importSettings;
    // The location the resource file is stored at on disk
    std::string resourceFilePath;
    // For resources that are loaded from a source file, IE a texture
    std::string sourcePath;
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

