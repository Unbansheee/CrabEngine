#include "Object.h"

#include "PropertySerializer.h"

const UID& Object::GetID() const
{
    return id;
}

void Object::Serialize(nlohmann::json& archive)
{
    auto& object_data = archive[id.to_string()];

    auto s = PropertySerializer();
    for (auto& prop : GetPropertiesFromThis())
    {
        prop.visit(s, object_data, this);
    }
}

void Object::Deserialize(nlohmann::json& archive)
{
    auto& object_data = archive[id.to_string()];
    
    auto s = PropertyDeserializer();
    for (auto& prop : GetPropertiesFromThis())
    {
        prop.visit(s, object_data, this);
    }
}
