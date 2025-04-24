module Engine.Resource.ImageTexture;
import Engine.Reflection.ClassDB;
import Engine.Resource.ResourceManager;
import Engine.Application;
import Engine.Resource.Texture;
import json;
import vfspp;


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
    LoadTextureFromPath(imageFilePath);
    TextureResource::LoadData();
}

void ImageTextureResource::LoadTextureFromPath(const std::filesystem::path& path)
{
    auto p = Application::Get().GetFilesystem()->OpenFile(path.string(), vfspp::IFile::FileMode::Read);
    p->Close();

    imageFilePath = path.string();
    texture = ResourceManager::loadTexture(p->GetFileInfo().AbsolutePath(), Application::Get().GetDevice(), &*view);
    width = texture->getWidth();
    height = texture->getHeight();

    loaded = true;
}
