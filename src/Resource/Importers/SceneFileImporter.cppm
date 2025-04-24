//
// Created by Vinnie on 24/04/2025.
//


module;
#include "ReflectionMacros.h"

export module Engine.Resource.Importer.SceneFileImporter;
import Engine.Resource.Importer;
import Engine.Resource.Importer.ImportSettings;
import Engine.Resource.Scene;
import Engine.Resource;

export class SceneFileImporter : public ResourceImporter {
    REGISTER_RESOURCE_IMPORTER(SceneFileImporter);

public:
    std::string GetResourceType() const override { return "Scene"; };

    std::unordered_set<std::string> GetSupportedExtensions() const override { return {".scene"}; };

    std::shared_ptr<Resource>
    Import(const std::filesystem::path &sourcePath, const ::ImportSettings &settings) override {
        auto scn = std::make_shared<SceneResource>();
        return scn;
    };

    std::shared_ptr<::ImportSettings> CreateDefaultSettings() const override {
        static std::shared_ptr<ImportSettings> s = std::make_shared<ImportSettings>();
        return s;
    };
};