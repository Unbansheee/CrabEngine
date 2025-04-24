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
    
    void Serialize(nlohmann::json& archive) override { Object::Serialize(archive); }
    void Deserialize(nlohmann::json& archive) override { Object::Deserialize(archive); }

    // Triggers resources such as mesh data, texture data, to be loaded into memory / gpu
    // The idea is that a resource can exist as a shell, holding metadata, and its sub-data will only be loaded when explicitly needed
    // To prevent having to keep big resources in memory, but still being able to reference them
    virtual void LoadData() { loaded = true; };
    virtual bool IsLoaded() const {return loaded;}
    void LoadIfRequired() const { if (!IsLoaded()) const_cast<Resource*>(this)->LoadData(); }


    virtual wgpu::raii::TextureView GetThumbnail();

    const std::string& GetName() const {return name;}
    bool IsSourceImported() const { return !bIsInline; }
    bool IsInline() const { return bIsInline; }
    const std::string& GetSourcePath() { return sourcePath; }
    const std::shared_ptr<ImportSettings>& GetImportSettings() { return importSettings; };
    std::string GetAbsoluteSourcePath() { return sourceInfo->AbsolutePath(); };

protected:

    std::string name;
    bool bIsInline = true;
    std::atomic<bool> loaded{false};
    std::shared_ptr<ImportSettings> importSettings;

    std::string sourcePath;
    std::optional<vfspp::FileInfo> sourceInfo;
private:
    static inline std::shared_ptr<TextureResource> DefaultResourceThumbnail = nullptr;

};

