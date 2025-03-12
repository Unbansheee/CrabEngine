#pragma once
#include "Mesh.h"

class OBJMeshResource : public MeshResource
{
    CRAB_CLASS(OBJMeshResource, MeshResource)
    BEGIN_PROPERTIES
        ADD_PROPERTY("Asset Path", meshAssetPath);
    END_PROPERTIES
public:
    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    void LoadOBJFromPath(const std::string& path);
    
    std::string meshAssetPath;
};
