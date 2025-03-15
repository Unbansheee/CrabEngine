
module object;
import reflection;
import property_serialization;
import uid;
import class_db;
import class_type;

const ClassType& Object::GetStaticClass()
{
    return ClassDB::Get().GetClass<Object>();
}

const UID& Object::GetID() const
{
    return id;
}

void Object::Serialize(nlohmann::json& archive)
{
    archive["class"] = GetStaticClassFromThis().Name;
    archive["uid"] = id.to_string();
    auto& properties = archive["properties"];

    auto s = PropertySerializer();
    for (auto& prop : GetPropertiesFromThis())
    {
        prop.visit(s, properties, this);
    }
}

void Object::Deserialize(nlohmann::json& archive)
{
    id = archive["uid"].get<std::string>();

    auto& properties = archive["properties"];
    auto s = PropertyDeserializer();
    for (auto& prop : GetPropertiesFromThis())
    {
        prop.visit(s, properties, this);
    }
}

void Object::OnPropertySet(Property& prop)
{
}
