module;
#include <fstream>
#include "json.hpp"

module scene_serializer;
//import json;


void SceneSerializer::SerializeScene(Node* rootNode, const std::filesystem::path& destination)
{
    nlohmann::json archive;
    
    rootNode->Serialize(archive);

    std::ofstream outFile(destination);
    outFile.clear();
    outFile << archive;
    outFile.close();
}

void SceneSerializer::DeserializeScene(Node* rootNode, const std::filesystem::path& scene)
{
    nlohmann::json archive;
    std::ifstream inFile(scene);
    inFile >> archive;
    inFile.close();

    rootNode->Deserialize(archive);
    /*
    for (auto node : archive)
    {
        std::cout << node;
    }
    */
}
