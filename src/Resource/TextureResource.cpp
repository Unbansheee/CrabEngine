module Engine.Resource.Texture;
/*
void TextureResource::InitializeFromData(wgpu::Device device, int width, int height, int channels, unsigned char* pixelData)
{
    texture = ResourceManager::loadTexture(device, width, height, channels, pixelData, &view);
    this->width = width;
    this->height = height;
}
*/

void TextureResource::LoadData()
{
    loaded = true;
}

wgpu::raii::TextureView TextureResource::GetThumbnail() {
    if (view) {
        return GetInternalTextureView();
    }

    return Resource::GetThumbnail();
}

