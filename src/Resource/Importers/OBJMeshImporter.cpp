
module Engine.Resource.Importer.OBJMesh;
import Engine.Resource.OBJMesh;

std::string OBJMeshImporter::GetResourceType() const
{
    return "OBJMeshResource";
}

std::unordered_set<std::string> OBJMeshImporter::GetSupportedExtensions() const
{
    return {".obj"};
}

std::shared_ptr<Resource> OBJMeshImporter::Import(const std::filesystem::path& sourcePath,
    const ResourceMetadata& settings)
{
    const auto& texSettings = static_cast<const OBJImportSettings&>(settings);
    auto tex = std::make_shared<OBJMeshResource>();
    return tex;
}

std::shared_ptr<ResourceMetadata> OBJMeshImporter::CreateDefaultSettings() const
{
    return std::make_shared<OBJImportSettings>();
}
