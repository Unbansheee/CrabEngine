module;

#pragma once
#include "ReflectionMacros.h"

export module obj_mesh_resource;
import mesh_resource;
import resource;

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
