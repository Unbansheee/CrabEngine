#pragma once
#include "TextureResource.h"

class ImageTextureResource : public TextureResource
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

    void LoadTextureFromPath(const std::string& path);
    
    std::string imageFilePath = {};
};
