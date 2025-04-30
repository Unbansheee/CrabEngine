module Engine.Object;
import Engine.Reflection;
import Engine.Reflection.Serialization;
import Engine.UID;
import Engine.Reflection.ClassDB;
import Engine.Reflection.Class;
import Engine.StringID;
import Engine.Application;

BaseObjectRegistrationObject::BaseObjectRegistrationObject()
{
    ClassDB::Get().RegisterClassType(Object::GetStaticClass());
    Object::RegisterMethods();
}

Object::~Object() {
    if (scriptInstance.has_value()) {
        Application::Get().GetScriptEngine()->CallManaged(L"Scripts.ScriptHost", L"DestroyScript", scriptInstance->ManagedHandle);
    }
}

void Object::AddFlag(uint64_t Flag) {
    ObjectFlags |= Flag;
}

bool Object::HasFlag(uint64_t Flag) {
    return ObjectFlags & Flag;
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
    archive["flags"] = ObjectFlags;
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
    if (archive.contains("flags")) {
        AddFlag(archive["flags"].get<uint64_t>());
    }

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
