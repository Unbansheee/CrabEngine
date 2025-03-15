module array_mesh_resource;
import json_conversions;

void ArrayMeshResource::Serialize(nlohmann::json& archive)
{
    Super::Serialize(archive);
    archive["is_runtime"] = bIsRuntime;
    if (!bIsRuntime)
    {
        archive["vertices"] = vertices;
        archive["indices"] = indices;
    }
}

void ArrayMeshResource::Deserialize(nlohmann::json& archive)
{
    Super::Deserialize(archive);
    archive.at("is_runtime").get_to<bool>(bIsRuntime);
    if (!bIsRuntime)
    {
        archive.at("vertices").get_to<std::vector<MeshVertex>>(vertices);
        archive.at("indices").get_to(indices);
    }
}

void ArrayMeshResource::LoadData()
{
    Super::LoadData();
    LoadFromMeshData(vertices, indices);

    loaded = true;

}
