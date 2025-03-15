
module obj_mesh_importer;
import obj_mesh_resource;

std::string OBJMeshImporter::GetResourceType() const
{
    return "OBJMeshResource";
}

std::unordered_set<std::string> OBJMeshImporter::GetSupportedExtensions() const
{
    return {".obj"};
}

std::shared_ptr<Resource> OBJMeshImporter::Import(const std::filesystem::path& sourcePath,
    const ImportSettings& settings)
{
    const auto& texSettings = static_cast<const OBJImportSettings&>(settings);
    auto tex = std::make_shared<OBJMeshResource>();
    tex->LoadOBJFromPath(sourcePath.string());
    return tex;
}

std::shared_ptr<ImportSettings> OBJMeshImporter::CreateDefaultSettings() const
{
    return std::make_shared<OBJImportSettings>();
}
