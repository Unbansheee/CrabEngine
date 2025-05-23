﻿module;

export module Engine.Resource.ImportManager;
import Engine.Resource.Importer;
import std;

export class ResourceMetadata;

export class ImportManager {
public:
    static ImportManager& Get()
    {
        static ImportManager s;
        return s;
    }
    
    void RegisterImporter(std::unique_ptr<ResourceImporter> importer);

    std::shared_ptr<Resource> Import(const std::filesystem::path& path);
    void Reimport(const std::shared_ptr<Resource>& resource);

    bool IsFileTypeImportable(std::filesystem::path extension) const;
private:
    std::vector<std::unique_ptr<ResourceImporter>> importers;
    std::unordered_map<std::string, ResourceImporter*> extensionToImporter;

    ResourceImporter* GetImporterForExtension(std::filesystem::path extension) const;
    std::shared_ptr<ResourceMetadata> LoadOrCreateImportSettings(std::filesystem::path::iterator::reference path, ResourceImporter* importer);

    std::shared_ptr<Resource> ImportSourceFile(const std::filesystem::path& path);
    void ReimportResource(const std::shared_ptr<Resource>& resource);

    ImportManager() = default;
};



