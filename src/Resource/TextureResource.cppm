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

    BIND_METHOD(void, LoadData);
    BIND_METHOD_OUTPARAM(Vector2U, GetSize);

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

    Vector2U GetSize() const { return Vector2U{width, height}; }


protected:
    int width = 0;
    int height = 0;
    
    wgpu::raii::Texture texture{};
    wgpu::raii::TextureView view{};

};
