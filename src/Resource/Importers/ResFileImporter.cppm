//
// Created by Vinnie on 27/03/2025.
//
module;
#include "ReflectionMacros.h"

export module Engine.Resource.Importer.ResFileImporter;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;

export class ResFileImporter : public ResourceImporter {
public:
    REGISTER_RESOURCE_IMPORTER(ResFileImporter)

    std::string GetResourceType() const override;
    std::unordered_set<std::string> GetSupportedExtensions() const override;
    std::shared_ptr<Resource> Import(const std::filesystem::path& sourcePath, const ImportSettings& settings) override;
    std::shared_ptr<ImportSettings> CreateDefaultSettings() const override;
};


