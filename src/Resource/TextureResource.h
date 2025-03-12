#pragma once
#include <filesystem>
#include <webgpu/webgpu.hpp>

#include "Resource.h"
#include "ResourceManager.h"


class TextureResource : public Resource
{
public:
    TextureResource() : Resource(){};
    void InitializeFromFile(const std::filesystem::path& file) override
    {
        texture = ResourceManager::loadTexture(file, Application::Get().GetDevice(), &view);
        width = texture.getWidth();
        height = texture.getHeight();
        filePath = file.string();
    }

    void InitializeFromData(wgpu::Device device, int width, int height, int channels, unsigned char* pixelData);
    
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

private:
};
