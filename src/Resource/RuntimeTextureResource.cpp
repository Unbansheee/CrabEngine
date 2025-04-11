module Engine.Resource.RuntimeTexture;
import Engine.Resource.ResourceManager;
import Engine.Application;


void RuntimeTextureResource::Serialize(nlohmann::json& archive)
{
    TextureResource::Serialize(archive);
}

void RuntimeTextureResource::Deserialize(nlohmann::json& archive)
{
    TextureResource::Deserialize(archive);
}

void RuntimeTextureResource::LoadData()
{
    TextureResource::LoadData();
    
}

void RuntimeTextureResource::LoadFromPixelData(int width, int height, int channels, unsigned char* pixelData)
{
    texture = ResourceManager::loadTexture(Application::Get().GetDevice(), width, height, channels, pixelData, &*view);
    this->width = width;
    this->height = height;

    loaded = true;
}

void RuntimeTextureResource::CreateBlankTexture(wgpu::TextureDescriptor descriptor)
{

    width = descriptor.size.width;
    height = descriptor.size.height;
    texture = Application::Get().GetDevice().createTexture(descriptor);
    
    wgpu::TextureViewDescriptor textureViewDesc{};
    textureViewDesc.aspect = wgpu::TextureAspect::All;
    textureViewDesc.baseArrayLayer = 0;
    textureViewDesc.arrayLayerCount = 1;
    textureViewDesc.baseMipLevel = 0;
    textureViewDesc.mipLevelCount = descriptor.mipLevelCount;
    textureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
    textureViewDesc.format = descriptor.format;
    view = texture->createView(textureViewDesc);

    loaded = true;
}
