module;

#pragma once

#include <filesystem>
#include <memory>
#include <unordered_set>

//#include "ImportSettings.h"

export module resource_importer;
import import_settings;

export class Resource;

export class ResourceImporter {
public:
    ResourceImporter() = default;
    virtual ~ResourceImporter();
    virtual std::string GetResourceType() const = 0;
    virtual std::unordered_set<std::string> GetSupportedExtensions() const = 0;
    virtual std::shared_ptr<Resource> Import(const std::filesystem::path& sourcePath, 
                               const ImportSettings& settings) = 0;
    virtual std::shared_ptr<ImportSettings> CreateDefaultSettings() const = 0;
};
