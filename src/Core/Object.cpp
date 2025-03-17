module object;
import reflection;
import property_serialization;
import uid;
import class_db;
import class_type;
import string_id;

BaseObjectRegistrationObject::BaseObjectRegistrationObject()
{
    ClassDB::Get().RegisterClassType(Object::GetStaticClass());
}

const ClassType& Object::GetStaticClass()
{
    static ClassType s
{
    .Name = MakeStringID("Object"),
    .Initializer = &Object::Create<Object>,
    .Properties = Object::GetClassProperties(),
    .Parent = MakeStringID("null")
    };
    
    return s;
}

const UID& Object::GetID() const
{
    return id;
}

void Object::Serialize(nlohmann::json& archive)
{
    archive["class"] = GetStaticClassFromThis().Name.string();
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

bool Object::IsA(const ClassType& type)
{
    return GetStaticClassFromThis().IsSubclassOf(type);
}
