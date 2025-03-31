module;

#pragma once
#include "ReflectionMacros.h"

export module Engine.Resource.Texture;
export import Engine.Resource;
import Engine.Resource.ResourceManager;
import Engine.WGPU;


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
        if (texture)
        {
            texture.release();
            view.release();
        }
    }

    wgpu::TextureView GetThumbnail() override;

    wgpu::Texture& GetInternalTexture() { return texture; }
    wgpu::TextureView& GetInternalTextureView() { return view; }
    const wgpu::Texture& GetInternalTexture() const { return texture; }
    const wgpu::TextureView& GetInternalTextureView() const { return view; }

protected:
    int width = 0;
    int height = 0;
    
    wgpu::Texture texture = nullptr;
    wgpu::TextureView view = nullptr;

};
