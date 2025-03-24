module;

#pragma once
#include "ReflectionMacros.h"

export module Engine.Resource.OBJMesh;
import Engine.Resource.Mesh;
import Engine.Resource;

export class OBJMeshResource : public MeshResource
{
    CRAB_CLASS(OBJMeshResource, MeshResource)
    BEGIN_PROPERTIES
    END_PROPERTIES
public:
    void Serialize(nlohmann::json& archive) override;
    void Deserialize(nlohmann::json& archive) override;
    void LoadData() override;

    void LoadOBJFromPath(const std::string& path);
};
