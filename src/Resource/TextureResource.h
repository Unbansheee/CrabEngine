#pragma once
#include <filesystem>
#include <webgpu/webgpu.hpp>

#include "Resource.h"
#include "ResourceManager.h"


class TextureResource : public Resource
{
public:
    TextureResource(const std::filesystem::path& path = ENGINE_RESOURCE_DIR "/null_texture_black.png")
    {
        texture = ResourceManager::loadTexture(path, Application::Get().GetDevice(), &view);
    }

    ~TextureResource() override
    {
        texture.release();
        view.release();
    }

    const wgpu::Texture& GetInternalTexture() const { return texture; }
    const wgpu::TextureView& GetInternalTextureView() const { return view; }

protected:
    int width = 0;
    int height = 0;

    std::string filePath;
    std::string name;

    wgpu::Texture texture = nullptr;
    wgpu::TextureView view = nullptr;
};
