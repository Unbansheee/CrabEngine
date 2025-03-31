module;

#pragma once
#include "ReflectionMacros.h"

export module Engine.Resource.RuntimeTexture;
export import Engine.Resource.Texture;
import Engine.Reflection.ClassDB;

export class RuntimeTextureResource : public TextureResource
{
public:
    CRAB_CLASS(RuntimeTextureResource, TextureResource)

    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    void LoadFromPixelData(int width, int height, int channels, unsigned char* pixelData);
    void CreateBlankTexture(wgpu::TextureDescriptor descriptor);
};
