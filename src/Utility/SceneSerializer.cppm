
module;
#pragma once

export module Engine.SceneSerializer;
import Engine.Node;
import std;

export struct SceneSerializer
{
    void SerializeScene(Node* rootNode, const std::filesystem::path& destination);
    std::unique_ptr<Node> DeserializeScene(const std::filesystem::path& scene);
};
