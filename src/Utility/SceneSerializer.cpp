module;
#include <fstream>
#include "json.hpp"

module Engine.SceneSerializer;
//import json;


void SceneSerializer::SerializeScene(Node* rootNode, const std::filesystem::path& destination)
{
    nlohmann::json archive;
    
    rootNode->Serialize(archive);

    std::ofstream outFile(destination);
    outFile.clear();
    outFile << std::setw(4) << archive << std::endl;
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
