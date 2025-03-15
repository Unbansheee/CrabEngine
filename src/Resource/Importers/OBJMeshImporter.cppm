module;

#pragma once
#include <filesystem>
#include <memory>
#include <unordered_set>

#include "ReflectionMacros.h"

export module obj_mesh_importer;
import import_manager;
import resource_importer;
import import_settings;
import class_db;
import <string>;

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
