module;
#include <filesystem>
#include <unordered_set>

#include "ReflectionMacros.h"

export module Engine.Resource.Importer.ShaderFile;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;

export class ShaderFileImporter : public ResourceImporter
{
    REGISTER_RESOURCE_IMPORTER(ShaderFileImporter)
public:
    std::string GetResourceType() const override;
    std::unordered_set<std::string> GetSupportedExtensions() const override;
    std::shared_ptr<Resource> Import(const ::std::filesystem::path& sourcePath, const ::ImportSettings& settings) override;
    std::shared_ptr<::ImportSettings> CreateDefaultSettings() const override;
};
