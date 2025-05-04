module Engine.Resource.Texture;

void TextureResource::LoadData()
{
    loaded = true;
}

wgpu::raii::TextureView TextureResource::GetThumbnail() {
    LoadIfRequired();
    if (view) {
        return GetInternalTextureView();
    }

    return Resource::GetThumbnail();
}

