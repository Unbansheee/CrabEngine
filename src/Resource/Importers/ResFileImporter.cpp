//
// Created by Vinnie on 27/03/2025.
//
module;
#include "json.hpp"

module Engine.Resource.Importer.ResFileImporter;
import std;
import Engine.Resource;
import Engine.Application;
import Engine.Filesystem;

std::string ResFileImporter::GetResourceType() const {
    return "Resource";
}

std::unordered_set<std::string> ResFileImporter::GetSupportedExtensions() const {
    return {".res"};
}

std::shared_ptr<Resource> ResFileImporter::Import(const std::filesystem::path &sourcePath,
    const ResourceMetadata &settings) {

    auto filePath = Filesystem::AbsolutePath(sourcePath.string());

    nlohmann::json j;
    std::ifstream inFile(filePath);
    inFile >> j;
    inFile.close();

    std::string type = j.at("class").get<std::string>();

    auto classData = ClassDB::Get().GetClassByName(type);
    if (!classData) return nullptr;

    auto r = static_cast<Resource*>(classData->Initializer());
    std::shared_ptr<Resource> resource;
    resource.reset(r);
    resource->Deserialize(j);

    resource->sourcePath = Filesystem::VirtualPath(sourcePath.string());
    resource->bIsInline = false;

    return resource;
}

std::shared_ptr<ResourceMetadata> ResFileImporter::CreateDefaultSettings() const {
    static std::shared_ptr<ResourceMetadata> s = std::make_shared<ResourceMetadata>();
    return s;
}