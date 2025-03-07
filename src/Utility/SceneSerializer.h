#pragma once
#include <filesystem>

#include "Nodes/Node.h"

struct SceneSerializer
{
    void SerializeScene(Node* rootNode, const std::filesystem::path& destination);
};
