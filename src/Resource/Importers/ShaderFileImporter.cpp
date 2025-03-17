
module shader_file_importer;
import shader_file_resource;
import resource;

std::string ShaderFileImporter::GetResourceType() const
{
    return "Shader File";
}

std::unordered_set<std::string> ShaderFileImporter::GetSupportedExtensions() const
{
    return {".wgsl"};
}

std::shared_ptr<Resource> ShaderFileImporter::Import(const std::filesystem::path& sourcePath,
    const ImportSettings& settings)
{
    const auto& texSettings = static_cast<const ImportSettings&>(settings);
    auto res = std::make_shared<ShaderFileResource>();
    res->shaderFilePath = sourcePath.string();
    return res;
}

std::shared_ptr<ImportSettings> ShaderFileImporter::CreateDefaultSettings() const
{
    return std::make_shared<ImportSettings>();
}
