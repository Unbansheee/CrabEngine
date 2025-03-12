#include "ImageTextureResource.h"

#include "Application.h"
#include "Core/ClassDB.h"



void ImageTextureResource::Serialize(nlohmann::json& archive)
{
    TextureResource::Serialize(archive);
}

void ImageTextureResource::Deserialize(nlohmann::json& archive)
{
    TextureResource::Deserialize(archive);
}

void ImageTextureResource::LoadData()
{
    assert(!imageFilePath.empty());
    
    LoadTextureFromPath(imageFilePath);
    assert(texture);
    
    TextureResource::LoadData();
}

void ImageTextureResource::LoadTextureFromPath(const std::string& path)
{
    imageFilePath = path;
    texture = ResourceManager::loadTexture(path, Application::Get().GetDevice(), &view);
    width = texture.getWidth();
    height = texture.getHeight();

    loaded = true;
}
