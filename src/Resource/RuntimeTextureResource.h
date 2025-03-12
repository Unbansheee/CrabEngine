#pragma once
#include "TextureResource.h"

class RuntimeTextureResource : public TextureResource
{
public:
    CRAB_CLASS(RuntimeTextureResource, TextureResource)

    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    void LoadFromPixelData(int width, int height, int channels, unsigned char* pixelData);
};
