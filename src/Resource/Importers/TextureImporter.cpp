module Engine.Resource.Importer.ImageTexture;
import Engine.Resource.ImageTexture;

std::string TextureImporter::GetResourceType() const
{
    return "Texture";
}

std::unordered_set<std::string> TextureImporter::GetSupportedExtensions() const
{
    return {".png", ".jpg"};
}

Resource::Ptr TextureImporter::Import(const std::filesystem::path& sourcePath, const ResourceMetadata& settings)
{
    auto tex = std::make_shared<ImageTextureResource>();
    tex->imageFilePath = sourcePath.string();
    return tex;
}

std::shared_ptr<ResourceMetadata> TextureImporter::CreateDefaultSettings() const
{
    return std::make_shared<TextureImportSettings>();
}
