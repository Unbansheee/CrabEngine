module;

#pragma once

#include "ReflectionMacros.h"

export module Engine.Resource.Importer.OBJMesh;
import Engine.Resource.ImportManager;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;
import Engine.Reflection.ClassDB;
import std;

export class OBJMeshImporter : public ResourceImporter
{
public:
    inline static AutoRegisterResourceImporter<OBJMeshImporter> ImporterRegistrationObject_OBJMeshImporter = AutoRegisterResourceImporter<OBJMeshImporter>();
    
    std::string GetResourceType() const override;
    std::unordered_set<std::string> GetSupportedExtensions() const override;
    std::shared_ptr<Resource> Import(const std::filesystem::path& sourcePath, const ImportSettings& settings) override;
    std::shared_ptr<ImportSettings> CreateDefaultSettings() const override;

    class OBJImportSettings : public ImportSettings {
        CRAB_CLASS(OBJImportSettings, ImportSettings)
        BEGIN_PROPERTIES
        END_PROPERTIES
        
    };
};
