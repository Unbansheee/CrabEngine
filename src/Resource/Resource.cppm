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

// A resource represents some kind of asset, such as a texture, a 3D model, a material.
// They can be imported from disk or created at runtime and saved to disk.
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

    // Get the resource's thumbnail (for editor use)
    virtual wgpu::raii::TextureView GetThumbnail();

    // Triggers resources such as mesh data, texture data, to be loaded into memory / gpu
    // The idea is that a resource can exist as a shell, holding metadata, and its sub-data will only be loaded when explicitly needed
    // To prevent having to keep big resources in memory, but still being able to reference them
    virtual void LoadData();
    // Has this resource's bulk data been loaded
    virtual bool IsLoaded() const;
    // Helper to load resource bulk data if it hasn't already been loaded
    void LoadIfRequired() const;
    // Get the name of this resource, usually corresponds to its filename
    const std::string& GetName() const;
    // Is imported from a file
    bool IsSourceImported() const;
    // Is created inline (ie. not imported from a file)
    bool IsInline() const;
    // Get virtual source path (eg /res/*)
    const std::string& GetSourcePath();
    // Get absolute source path (path on disk (eg C:/Stuff/Things))
    const std::string& GetAbsolutePath();;

    // TODO: Probably remove this
    // Get the imported metadata for this resource
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

