module;
#include "json.hpp"

module Engine.Resource.ImportManager;
import Engine.Resource.ResourceManager;
import Engine.Resource;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;

void ImportManager::RegisterImporter(std::unique_ptr<ResourceImporter> importer)
{
    importers.push_back(std::move(importer));
}



std::shared_ptr<Resource> ImportManager::Import(const std::filesystem::path& path)
{
    return ImportSourceFile(path);
}

bool ImportManager::IsFileTypeImportable(std::filesystem::path extension) const {
    return GetImporterForExtension(extension) != nullptr;
}

ResourceImporter* ImportManager::GetImporterForExtension(std::filesystem::path extension) const
{
    for (auto& imp : importers)
    {
        if (imp->GetSupportedExtensions().contains(extension.string()))
        {
            return imp.get();
        }
    }
    return nullptr;
}

std::shared_ptr<ImportSettings> ImportManager::LoadOrCreateImportSettings(const std::filesystem::path& path, ResourceImporter* importer)
{
    auto importsettingPath = (path.string() + ".meta");
    auto setting = importer->CreateDefaultSettings();
    if (std::filesystem::exists(importsettingPath))
    {
        nlohmann::json j;
        std::ifstream inFile(importsettingPath);
        inFile >> j;
        setting->Deserialize(j);
    }
    else
    {
        nlohmann::json j;
        std::ofstream outFile(importsettingPath);
        setting->Serialize(j);
        outFile << j;
        outFile.close();
    }
    return setting;
}

std::shared_ptr<Resource> ImportManager::ImportSourceFile(const std::filesystem::path& path)
{
    auto importer = GetImporterForExtension(path.extension());
    auto settings = LoadOrCreateImportSettings(path, importer);
    auto res = importer->Import(path, *settings);
    if (!res) return nullptr;
    if (settings) res->importSettings = settings;
    res->sourcePath = path.string();
    res->bIsInline = false;
    res->name = path.stem().string();
    res->LoadData();
    return res;
}
