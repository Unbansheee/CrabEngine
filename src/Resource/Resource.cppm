module;

#pragma once

#include <filesystem>
#include "ReflectionMacros.h"
//#include "Importers/ResourceImporter.cppm"
//#include "Utility/ObservableDtor.h"

export module Engine.Resource;
export import Engine.Object;
export import Engine.Object.ObservableDtor;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;
import Engine.Reflection.Class;
import Engine.WGPU;

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
        ADD_PROPERTY("Name", name)
        ADD_PROPERTY_FLAGS("SourcePath", sourcePath, PropertyFlags::HideFromInspector)
    END_PROPERTIES

    using Ptr = std::shared_ptr<Resource>;
    
    void Serialize(nlohmann::json& archive) override { Object::Serialize(archive); }
    void Deserialize(nlohmann::json& archive) override { Object::Deserialize(archive); }
    virtual void LoadData() { loaded = true; };  // For explicit loading
    virtual bool IsLoaded() const {return loaded;}

    virtual wgpu::TextureView GetThumbnail();

    const std::string& GetName() const {return name;}
    bool IsSourceImported() const { return !bIsInline; }
    bool IsInline() const { return bIsInline; }
    const std::string& GetSourcePath() {return sourcePath; }
    const std::shared_ptr<ImportSettings>& GetImportSettings() { return importSettings; };

protected:
    std::string name;
    bool bIsInline = true;
    std::string sourcePath;
    std::atomic<bool> loaded{false};
    std::shared_ptr<ImportSettings> importSettings;

private:
    static inline std::shared_ptr<TextureResource> DefaultResourceThumbnail = nullptr;

};

