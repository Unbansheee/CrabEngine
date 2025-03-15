module;

#pragma once
#include <filesystem>
#include "ReflectionMacros.h"

export module image_texture_resource;
export import texture_resource;

export class ImageTextureResource : public TextureResource
{
public:
    CRAB_CLASS(ImageTextureResource, TextureResource)
    CLASS_FLAG(EditorVisible)

    BEGIN_PROPERTIES
        ADD_PROPERTY("Texture Path", imageFilePath)
    END_PROPERTIES
    
    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    void LoadTextureFromPath(const std::filesystem::path& path);
    
    std::string imageFilePath = {};
};
