module;

#pragma once
#include "ReflectionMacros.h"

export module runtime_texture_resource;
export import texture_resource;
import class_db;

export class RuntimeTextureResource : public TextureResource
{
public:
    CRAB_CLASS(RuntimeTextureResource, TextureResource)

    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    void LoadFromPixelData(int width, int height, int channels, unsigned char* pixelData);
};
