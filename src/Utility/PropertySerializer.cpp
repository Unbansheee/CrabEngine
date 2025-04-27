//#include "PropertySerializer.h"
module;
#include "json.hpp"

module Engine.Reflection.Serialization;
import Engine.Resource;
import Engine.Resource.ResourceManager;
import Engine.Node;
import Engine.Transform;
import std;
import Engine.Reflection.ClassDB;


//#include "glm/gtc/quaternion.hpp"

// Serializer
void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, int& val)
{
    auto& a = *archive;
    a[prop.name()] = val;
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, float& val)
{
    auto& a = *archive;
    a[prop.name()] = val;
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, bool& val)
{
    auto& a = *archive;
    a[prop.name()] = val;
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, std::string& val)
{
    auto& a = *archive;
    a[prop.name()] = val;
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector2& val)
{
    auto& a = *archive;
    a[prop.name()] = {val.x, val.y};
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector3& val)
{
    auto& a = *archive;\
    a[prop.name()] = {val.x, val.y, val.z};
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector4& val)
{
    auto& a = *archive;
    a[prop.name()] = {val.x, val.y, val.z, val.w};
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, Quat& val)
{
    auto& a = *archive; a[prop.name()] = { val.w, val.x, val.y, val.z};
}

void PropertySerializer::operator()(PropertyView &prop, nlohmann::json *archive, UID &val) {
    auto& a = *archive; a[prop.name()] = val.to_string();
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, Transform& val)
{
    auto& a = *archive;
    a[prop.name()] = {
        {val.Position.x, val.Position.y, val.Position.z},
        {val.Orientation.w, val.Orientation.x, val.Orientation.y, val.Orientation.z},
        {val.Scale.x, val.Scale.y, val.Scale.z}};
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, std::shared_ptr<Resource>& val)
{
    auto& a = *archive;
    auto& properties = a[prop.name()];
    if (val)
    {
        if (val->IsInline())
        {
            properties["import_type"] = "inline";
            auto& inlineDef = properties["inline_resource"];
            val->Serialize(inlineDef);
        }
        else {
            properties["import_type"] = "file";
            properties["source_file_path"] = val->GetSourcePath();
            properties["uid"] = val->GetID().to_string();
            ResourceManager::SaveResource(val);
        }
    }
    else
    {
        properties["import_type"] = "";
        properties["source_file_path"] = "";
    }
}

void PropertySerializer::operator()(PropertyView &prop, nlohmann::json *archive, ObjectRef<Object> &val) {
}


// Deserializer

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, int& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    val = a[prop.name()].get<int>();
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, float& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    val = (a[prop.name()].template get<float>());
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, bool& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    val = (a[prop.name()].template get<bool>());
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, std::string& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    val = (a[prop.name()].get<std::string>());
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector2& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    std::array<float, 2> v = a[prop.name()].get<std::array<float, 2>>();
    val = {v[0], v[1]};
    prop.set<Vector2>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector3& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    std::array<float, 3> v = a[prop.name()].get<std::array<float, 3>>();
    val = {v[0], v[1], v[2]};
    prop.set<Vector3>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector4& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    std::array<float, 4> v = a[prop.name()].get<std::array<float, 4>>();
    val = {v[0], v[1], v[2], v[3]};
    prop.set<Vector4>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Quat& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    std::array<float, 4> v = a[prop.name()].get<std::array<float, 4>>();
    val = {v[0], v[1], v[2], v[3]};
    prop.set<Quat>(val);
}

void PropertyDeserializer::operator()(PropertyView &prop, nlohmann::json *archive, UID &val) {
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    auto v = a.at(prop.name());
    auto idstring = v.get<std::string>();
    val = UID(idstring);

    prop.set<UID>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Transform& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;

    std::array<std::vector<float>, 3> v = a[prop.name()].get<std::array<std::vector<float>, 3>>();
    val.Position = {v[0][0], v[0][1], v[0][2]};
    val.Orientation = {v[1][0], v[1][1], v[1][2], v[1][3]};
    val.Scale = {v[2][0], v[2][1], v[2][2]};

    prop.set<Transform>(val);
    if (prop.objectClass && prop.objectClass->IsSubclassOf(Node::GetStaticClass())) {
        auto n = static_cast<Node*>(prop.object);
        n->UpdateTransform();
    }
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, std::shared_ptr<Resource>& val)
{
    auto& a = *archive;
    if (!a.contains(prop.name())) return;
    
    auto& properties = a[prop.name()];
    
    bool isInline = properties.at("import_type").get<std::string>() == "inline";
    bool isSource = properties.at("import_type").get<std::string>() == "file";
    if (!isInline && !isSource) return;

    if (isInline)
    {
        auto& j = properties["inline_resource"];
        auto res = static_cast<Resource*>(ClassDB::Get().GetClassByName(j.at("class"))->Initializer());
        std::shared_ptr<Resource> resource;
        resource.reset(res);
        resource->Deserialize(j);
        val = resource;
        prop.set(val);
        return;
    }

    // load from file
    std::string source_path = properties.at("source_file_path").get<std::string>();
    std::string uidString = properties.at("uid").get<std::string>();
    UID id = uidString;
    auto r = ResourceManager::FindByID(id);
    if (!r) {
        r = ResourceManager::Load(source_path);
    }
    val = r;
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView &prop, nlohmann::json *archive, ObjectRef<Object> &val) {
}
