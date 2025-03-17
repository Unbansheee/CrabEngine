module;
#include <filesystem>
#include <unordered_set>

#include "ReflectionMacros.h"

export module shader_file_importer;
import resource_importer;
import import_settings;

export class ShaderFileImporter : public ResourceImporter
{
    REGISTER_RESOURCE_IMPORTER(ShaderFileImporter)
public:
    std::string GetResourceType() const override;
    std::unordered_set<std::string> GetSupportedExtensions() const override;
    std::shared_ptr<Resource> Import(const ::std::filesystem::path& sourcePath, const ::ImportSettings& settings) override;
    std::shared_ptr<::ImportSettings> CreateDefaultSettings() const override;
};
