#include "RuntimeTextureResource.h"

#include "Application.h"
#include "Core/ClassDB.h"


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
    texture = ResourceManager::loadTexture(Application::Get().GetDevice(), width, height, channels, pixelData, &view);
    this->width = width;
    this->height = height;

    loaded = true;
}
