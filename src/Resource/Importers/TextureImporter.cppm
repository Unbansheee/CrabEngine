module;

#pragma once

#include <filesystem>
#include <unordered_set>

#include "ReflectionMacros.h"
//#include "webgpu/webgpu.hpp"

export module texture_importer;
import resource_importer;
import import_manager;
import import_settings;
import wgpu;
import resource;
import class_type;

class TextureImporter : public ResourceImporter
{
public:
    REGISTER_RESOURCE_IMPORTER(TextureImporter)
    
    std::string GetResourceType() const override;
    std::unordered_set<std::string> GetSupportedExtensions() const override;
    std::shared_ptr<Resource> Import(const std::filesystem::path& sourcePath, const ImportSettings& settings) override;
    std::shared_ptr<ImportSettings> CreateDefaultSettings() const override;

    class TextureImportSettings : public ImportSettings {
        CRAB_CLASS(TextureImportSettings, ImportSettings)
        BEGIN_PROPERTIES
            //ADD_PROPERTY("TextureFormat", format)
            ADD_PROPERTY("GenerateMipmaps", generateMipmaps)
            ADD_PROPERTY("sRGB", sRGB)
        END_PROPERTIES
        
        wgpu::TextureFormat format = wgpu::TextureFormat::RGBA8Unorm;
        bool generateMipmaps = true;
        bool sRGB = true;
    };
};
