module;

#pragma once
#include "ReflectionMacros.h"

export module Engine.Resource.Texture;
export import Engine.Resource;
import Engine.Resource.ResourceManager;
import Engine.WGPU;
import Engine.Types;


export class TextureResource : public Resource
{
public:
    CRAB_CLASS(TextureResource, Resource)

    TextureResource() : Resource()
    {
    };

    void LoadData() override;

    ~TextureResource() override
    {
    }

    wgpu::raii::TextureView GetThumbnail() override;

    wgpu::raii::Texture& GetInternalTexture() {
        LoadIfRequired();
        return texture;
    }

    wgpu::raii::TextureView& GetInternalTextureView() {
        LoadIfRequired();
        return view;
    }
    const wgpu::raii::Texture& GetInternalTexture() const {
        LoadIfRequired();
        return texture;
    }
    const wgpu::raii::TextureView& GetInternalTextureView() const {
        LoadIfRequired();
        return view;
    }

    glm::uvec2 GetSize() const {return {width, height};}



protected:
    int width = 0;
    int height = 0;
    
    wgpu::raii::Texture texture{};
    wgpu::raii::TextureView view{};

};
