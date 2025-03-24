module;

#pragma once

#include <filesystem>
#include <unordered_set>

#include "ReflectionMacros.h"
//#include "webgpu/webgpu.hpp"

export module Engine.Resource.Importer.ImageTexture;
import Engine.Resource.Importer;
import Engine.Resource.ImportManager;
import Engine.Resource.Importer.ImportSettings;
import Engine.WGPU;
import Engine.Resource;
import Engine.Reflection.Class;

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
