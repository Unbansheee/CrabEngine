//
// Created by Vinnie on 27/03/2025.
//
module;
#include "json.hpp"

module Engine.Resource.Importer.ResFileImporter;
import std;
import Engine.Resource;

std::string ResFileImporter::GetResourceType() const {
    return "Resource";
}

std::unordered_set<std::string> ResFileImporter::GetSupportedExtensions() const {
    return {".res"};
}

std::shared_ptr<Resource> ResFileImporter::Import(const std::filesystem::path &sourcePath,
    const ImportSettings &settings) {
    nlohmann::json j;
    std::ifstream inFile(sourcePath);
    inFile >> j;
    inFile.close();

    std::string type = j.at("class").get<std::string>();

    auto classData = ClassDB::Get().GetClassByName(type);
    auto r = static_cast<Resource*>(classData->Initializer());
    std::shared_ptr<Resource> resource;
    resource.reset(r);
    resource->Deserialize(j);
    resource->sourcePath = sourcePath.string();
    resource->bIsInline = false;

    return resource;
}

std::shared_ptr<ImportSettings> ResFileImporter::CreateDefaultSettings() const {
    static std::shared_ptr<ImportSettings> s = std::make_shared<ImportSettings>();
    return s;
}