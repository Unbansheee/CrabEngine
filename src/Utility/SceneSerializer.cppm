
module;
#pragma once

export module scene_serializer;
import node;
import std;

export struct SceneSerializer
{
    void SerializeScene(Node* rootNode, const std::filesystem::path& destination);
    void DeserializeScene(Node* rootNode, const std::filesystem::path& scene);
};
