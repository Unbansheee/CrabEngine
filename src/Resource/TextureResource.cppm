module;

#pragma once
#include <filesystem>
//#include <webgpu/webgpu.hpp>
#include "ReflectionMacros.h"

export module texture_resource;
export import resource;
import resource_manager;
import wgpu;


export class TextureResource : public Resource
{
public:
    CRAB_CLASS(TextureResource, Resource)
    CLASS_FLAG(EditorVisible)

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

    const wgpu::Texture& GetInternalTexture() const { return texture; }
    const wgpu::TextureView& GetInternalTextureView() const { return view; }

protected:
    int width = 0;
    int height = 0;
    
    wgpu::Texture texture = nullptr;
    wgpu::TextureView view = nullptr;

private:
};
