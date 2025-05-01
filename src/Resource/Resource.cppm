module;

#pragma once

#include "ReflectionMacros.h"

export module Engine.Resource;
export import Engine.Object;
export import Engine.Object.ObservableDtor;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;
import Engine.Reflection.Class;
import Engine.WGPU;
import vfspp;
import rocket;
import Engine.Filesystem;

export class ImportManager;
export class TextureResource;

export class Resource : public Object
{
    friend class ResourceManager;
    friend class ImportManager;
    friend class ResFileImporter;
    
public:
    CRAB_CLASS(Resource, Object)

    BEGIN_PROPERTIES
        ADD_PROPERTY_FLAGS("Name", name, PropertyFlags::HideFromInspector)
        ADD_PROPERTY_FLAGS("SourcePath", sourcePath, PropertyFlags::HideFromInspector)
    END_PROPERTIES

    using Ptr = std::shared_ptr<Resource>;
    
    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    virtual ~Resource();

    // Triggers resources such as mesh data, texture data, to be loaded into memory / gpu
    // The idea is that a resource can exist as a shell, holding metadata, and its sub-data will only be loaded when explicitly needed
    // To prevent having to keep big resources in memory, but still being able to reference them
    virtual void LoadData();
    virtual bool IsLoaded() const;
    void LoadIfRequired() const;
    virtual wgpu::raii::TextureView GetThumbnail();
    const std::string& GetName() const;
    bool IsSourceImported() const;
    bool IsInline() const;
    const std::string& GetSourcePath();
    const std::string& GetAbsolutePath();;
    const std::shared_ptr<ResourceMetadata>& GetImportSettings();;

    rocket::signal<void()> OnResourceSaved;
protected:

    std::string name;
    bool bIsInline = true;
    std::atomic<bool> loaded{false};
    std::shared_ptr<ResourceMetadata> importSettings;
    std::string sourcePath{};

    // Runtime only, not serialized
    std::string absolutePath{};
    std::filesystem::file_time_type resource_time;
private:
    static inline std::shared_ptr<TextureResource> DefaultResourceThumbnail = nullptr;

    // C# Script lifetime management
    static void ReleaseResource(Resource* res);
    BIND_STATIC_METHOD(void, ReleaseResource);

    static Resource* LoadResource(const char* path);
    BIND_STATIC_METHOD(Resource*, LoadResource)
};

