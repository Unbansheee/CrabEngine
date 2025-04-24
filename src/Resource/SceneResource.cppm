//
// Created by Vinnie on 22/04/2025.
//

module;
#include "ReflectionMacros.h"

export module Engine.Resource.Scene;
import Engine.Resource;
import Engine.Node;

export class SceneResource : public Resource {
     CRAB_CLASS(SceneResource, Resource)

public:
     wgpu::raii::TextureView GetThumbnail() override;
     std::unique_ptr<Node> Instantiate();
     void LoadData() override;
     void Serialize(nlohmann::json &archive) override;
private:
     inline static std::shared_ptr<TextureResource> SceneResourceThumbnail {};

     std::unique_ptr<Node> BaseNode;
};

