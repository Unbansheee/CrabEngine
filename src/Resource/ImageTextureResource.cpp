module Engine.Resource.ImageTexture;
import Engine.Reflection.ClassDB;
import Engine.Resource.ResourceManager;
import Engine.Application;
import Engine.Resource.Texture;
import json;


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
    //assert(!imageFilePath.empty());
    
    LoadTextureFromPath(imageFilePath);
    //assert(texture);
    
    TextureResource::LoadData();
}

void ImageTextureResource::LoadTextureFromPath(const std::filesystem::path& path)
{
    imageFilePath = path.string();
    texture = ResourceManager::loadTexture(path, Application::Get().GetDevice(), &*view);
    width = texture->getWidth();
    height = texture->getHeight();

    loaded = true;
}
