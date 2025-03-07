#include "SceneSerializer.h"

#include <fstream>

void SceneSerializer::SerializeScene(Node* rootNode, const std::filesystem::path& destination)
{
    nlohmann::json archive;
    
    rootNode->Serialize(archive);

    std::ofstream outFile(destination);
    outFile.clear();
    outFile << archive;
    outFile.close();
}
