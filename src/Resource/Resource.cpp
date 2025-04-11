module Engine.Resource;
import Engine.Resource.ResourceManager;
import Engine.Resource.Texture;

wgpu::raii::TextureView Resource::GetThumbnail() {
    if (!DefaultResourceThumbnail) {
        DefaultResourceThumbnail = ResourceManager::Load<TextureResource>(ENGINE_RESOURCE_DIR"/Textures/T_DefaultResourceThumbnail.png");
    }
    return DefaultResourceThumbnail->GetInternalTextureView();
}
