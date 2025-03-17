//#include "PropertySerializer.h"
#include "json.hpp"

module property_serialization;
import resource;
import resource_manager;
import node;
import transform;
import <string>;


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

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, Transform& val)
{
    auto& a = *archive;
    a[prop.name()] = {
        {val.Position.x, val.Position.y, val.Position.z},
        {val.Orientation.w, val.Orientation.x, val.Orientation.y, val.Orientation.z},
        {val.Scale.x, val.Scale.y, val.Scale.z}};
}

void PropertySerializer::operator()(PropertyView& prop, nlohmann::json* archive, StrongResourceRef& val)
{
    auto& a = *archive;
    auto& properties = a[prop.name()];
    if (auto res = val.Get<Resource>())
    {
        properties["is_source_imported"] = res->IsSourceImported();
        properties["resource_file_path"] = res->GetResourcePath();
        properties["source_file_path"] = res->GetSourcePath();
    }
    else
    {
        properties["is_source_imported"] = false;
        properties["resource_file_path"] = "";
        properties["source_file_path"] = "";
    }

    
    /*
    properties["resource_path"] = val.GetResourcePath();
    if (auto res = val.Get<Resource>())
    {
        auto& resource_data = properties["resource_data"];
        res->Serialize(resource_data);
    }
    */
}


// Deserializer

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, int& val)
{
    auto& a = *archive;
    val = a[prop.name()].get<int>();
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, float& val)
{
    auto& a = *archive;
    val = (a[prop.name()].template get<float>());
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, bool& val)
{
    auto& a = *archive;
    val = (a[prop.name()].template get<bool>());
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, std::string& val)
{
    auto& a = *archive;
    val = (a[prop.name()].get<std::string>());
    prop.set(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector2& val)
{
    auto& a = *archive;
    std::array<float, 2> v = a[prop.name()].get<std::array<float, 2>>();
    val = {v[0], v[1]};
    prop.set<Vector2>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector3& val)
{
    auto& a = *archive;
    std::array<float, 3> v = a[prop.name()].get<std::array<float, 3>>();
    val = {v[0], v[1], v[2]};
    prop.set<Vector3>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Vector4& val)
{
    auto& a = *archive;
    std::array<float, 4> v = a[prop.name()].get<std::array<float, 4>>();
    val = {v[0], v[1], v[2], v[3]};
    prop.set<Vector4>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Quat& val)
{
    auto& a = *archive;
    std::array<float, 4> v = a[prop.name()].get<std::array<float, 4>>();
    val = {v[0], v[1], v[2], v[3]};
    prop.set<Quat>(val);
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, Transform& val)
{
    auto& a = *archive;
    std::array<std::vector<float>, 3> v = a[prop.name()].get<std::array<std::vector<float>, 3>>();
    val.Position = {v[0][0], v[0][1], v[0][2]};
    val.Orientation = {v[1][0], v[1][1], v[1][2], v[1][3]};
    val.Scale = {v[2][0], v[2][1], v[2][2]};

    prop.set<Transform>(val);
    if (auto n = dynamic_cast<Node*>(prop.object))
    {
        n->UpdateTransform();
    }
}

void PropertyDeserializer::operator()(PropertyView& prop, nlohmann::json* archive, StrongResourceRef& val)
{
    auto& a = *archive;
    auto& properties = a[prop.name()];
    
    bool sourceImported =  properties.at("is_source_imported").get<bool>();
    std::string resource_file_path = properties.at("resource_file_path").get<std::string>();
    std::string source_path = properties.at("source_file_path").get<std::string>();
    
    /*
    if (val)
    {
        if (auto res = val.Get<Resource>())
        {
            res->Deserialize(a);
            val = res;
            prop.set(val);
            return;
        }
    }
    */

    auto& load_path = sourceImported ? source_path : resource_file_path;
    if (!load_path.empty())
    {
        auto newResource = ResourceManager::Load(load_path);
        val = newResource;
        prop.set(val);
    }
}
