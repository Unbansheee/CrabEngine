#pragma once
#include <filesystem>
#include <webgpu/webgpu.hpp>

#include "Resource.h"
#include "ResourceManager.h"
#include "Core/ClassDB.h"


class TextureResource : public Resource
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
