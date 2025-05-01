module Engine.Resource;
import Engine.Resource.ResourceManager;
import Engine.Resource.Texture;

std::unordered_map<Resource*, std::shared_ptr<Resource>> g_ResourceLifetimeMap;

wgpu::raii::TextureView Resource::GetThumbnail() {
    if (!DefaultResourceThumbnail) {
        DefaultResourceThumbnail = ResourceManager::Load<TextureResource>("/engine/Textures/T_DefaultResourceThumbnail.png");
    }
    return DefaultResourceThumbnail->GetInternalTextureView();
}

void Resource::ReleaseResource(Resource* res) {
    g_ResourceLifetimeMap.erase(res);
}

Resource * Resource::LoadResource(const char *path) {
    auto res = ResourceManager::Load(path);
    if (!res) return nullptr;

    g_ResourceLifetimeMap.insert({res.get(), res});
    return res.get();
}
