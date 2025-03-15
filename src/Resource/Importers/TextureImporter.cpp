module texture_importer;
import image_texture_resource;

std::string TextureImporter::GetResourceType() const
{
    return "Texture";
}

std::unordered_set<std::string> TextureImporter::GetSupportedExtensions() const
{
    return {".png", ".jpg"};
}

Resource::Ptr TextureImporter::Import(const std::filesystem::path& sourcePath, const ImportSettings& settings)
{
    const auto& texSettings = static_cast<const TextureImportSettings&>(settings);
    auto tex = std::make_shared<ImageTextureResource>();
    tex->LoadTextureFromPath(sourcePath);
    return tex;
}

std::shared_ptr<ImportSettings> TextureImporter::CreateDefaultSettings() const
{
    return std::make_shared<TextureImportSettings>();
}
