module;
#include <fstream>
#include "json.hpp"

module Engine.SceneSerializer;
import Engine.Reflection.ClassDB;
import Engine.Assert;
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

std::unique_ptr<Node> SceneSerializer::DeserializeScene(const std::filesystem::path& scene)
{
    nlohmann::json archive;
    std::ifstream inFile(scene);
    inFile >> archive;
    inFile.close();

    // only one child as the scene root
    auto childType = archive.at("class").get<std::string>();
    auto classType = ClassDB::Get().GetClassByName(childType);
    if (classType->Initializer) {
        Object* n = classType->Initializer();
        Node* node = static_cast<Node*>(n);
        auto instance = Node::InitializeNode(node, classType->Name.string());
        instance->Deserialize(archive);
        return instance;
    }


    Assert::Fail("Scene import has no root");
    return {};
    //rootNode->Deserialize(archive);
    /*
    for (auto node : archive)
    {
        std::cout << node;
    }
    */
}
