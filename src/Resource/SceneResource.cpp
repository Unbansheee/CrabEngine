//
// Created by Vinnie on 24/04/2025.
//

module Engine.Resource.Scene;
import Engine.Resource.ResourceManager;
import Engine.Resource.ImageTexture;
import Engine.Node;
import Engine.SceneSerializer;

wgpu::raii::TextureView SceneResource::GetThumbnail() {
    if (!SceneResourceThumbnail) {
        SceneResourceThumbnail = ResourceManager::Load<ImageTextureResource>("/engine/Textures/T_SceneThumbnail.png");
    }
    return SceneResourceThumbnail->GetInternalTextureView();
}

std::unique_ptr<Node> SceneResource::Instantiate() {
    LoadIfRequired();
    return BaseNode->Duplicate();
}

void SceneResource::LoadData() {
    SceneSerializer s;
    BaseNode = Node::NewNode("SceneRoot");
    s.DeserializeScene(BaseNode.get(), sourceInfo->AbsolutePath());

    Resource::LoadData();
}

void SceneResource::Serialize(nlohmann::json &archive) {
    Resource::Serialize(archive);
}
