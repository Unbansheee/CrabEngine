module;

#pragma once
#include <filesystem>
#include <memory>
#include <unordered_map>


export module Engine.Resource.ImportManager;
import Engine.Resource.Importer;

export class ImportSettings;

export class ImportManager {
public:
    static ImportManager& Get()
    {
        static ImportManager s;
        return s;
    }
    
    void RegisterImporter(std::unique_ptr<ResourceImporter> importer);
    bool IsEngineResource(std::filesystem::path::iterator::reference path);

    std::shared_ptr<Resource> ImportOrLoad(const std::filesystem::path& path);

private:
    std::vector<std::unique_ptr<ResourceImporter>> importers;
    std::unordered_map<std::string, ResourceImporter*> extensionToImporter;

    ResourceImporter* GetImporterForExtension(std::filesystem::path extension);
    std::shared_ptr<ImportSettings> LoadOrCreateImportSettings(std::filesystem::path::iterator::reference path, ResourceImporter* importer);

    std::shared_ptr<Resource> ImportSourceFile(const std::filesystem::path& path);

    ImportManager() = default;
};



