#pragma once
import Engine.Reflection;
import Engine.Reflection.ClassDB;
import Engine.Reflection.Class;
import Engine.Reflection.AutoRegistration;
import Engine.Variant;

#define GET_PROPERTY_NAME(Property)\
    #Property

#define BEGIN_PROPERTIES \
virtual const std::vector<Property>& GetPropertiesFromThis() override { return GetClassProperties(); }\
static const auto& GetClassProperties() { \
static const std::vector<Property> props = []{ \
std::vector<Property> base = Super::GetClassProperties(); \
std::vector<Property> custom;



#define BEGIN_STRUCT_PROPERTIES(Struct) \
using ThisClass = Struct; \
constexpr static inline std::string_view ClassName = #Struct; \
static const std::vector<Property>& GetStructProperties() { \
static const auto props = []{ \
std::vector<Property> custom;


#define END_STRUCT_PROPERTIES \
return custom; \
}(); \
return props; \
}



#define ADD_PROPERTY_FLAGS(DisplayName, Member, Flags) \
custom.emplace_back( \
#Member, \
DisplayName, \
std::string(ClassName), \
&ThisClass::Member, \
Flags, \
&ThisClass::StaticOnPropertySet \
);

#define ADD_NESTED_STRUCT(StructMember, StructType) \
{ \
const auto& nested_props = StructType::GetStructProperties(); \
for (const auto& nested_prop : nested_props) { \
const std::string full_name = std::string(#StructMember) + "." + nested_prop.name; \
\
auto getter = [=](void* obj) -> ValueVariant { \
ThisClass* parent = static_cast<ThisClass*>(obj); \
void* struct_ptr = static_cast<void*>(&(parent->StructMember)); \
return nested_prop.getVariant(struct_ptr); \
}; \
\
auto setter = [=](void* obj, const ValueVariant& value) { \
ThisClass* parent = static_cast<ThisClass*>(obj); \
void* struct_ptr = static_cast<void*>(&(parent->StructMember)); \
nested_prop.setVariant(struct_ptr, value); \
}; \
\
custom.emplace_back(Property( \
full_name, \
nested_prop.displayName, \
#StructType, \
std::function<ValueVariant(void*)>(getter), \
std::function<void(void*, const ValueVariant&)>(setter), \
&ThisClass::StaticOnPropertySet, \
nested_prop.flags \
)); \
} \
}

#define ADD_STRUCT_PROPERTY(DisplayName, Member) \
custom.emplace_back(\
#Member, \
DisplayName, \
"Struct Property", \
&ThisClass::Member, \
PropertyFlags::None \
);

#define ADD_STRUCT_PROPERTY_FLAGS(DisplayName, Member, Flags) \
custom.emplace_back(\
#Member, \
DisplayName, \
"Struct Property", \
&ThisClass::Member, \
Flags \
);

#define ADD_PROPERTY(DisplayName, Member) \
custom.emplace_back( \
#Member, \
DisplayName, \
std::string(ClassName), \
&ThisClass::Member, \
PropertyFlags::None, \
&ThisClass::StaticOnPropertySet\
);

#define END_PROPERTIES \
base.insert(base.end(), custom.begin(), custom.end()); \
return base; \
}(); \
return props; \
}\

// Usage: Place this in .cpp files
#define CRAB_CLASS(Class, ParentClass) \
public:\
constexpr static inline std::string_view ClassName = #Class; \
using ThisClass = Class; \
using Super = ParentClass; \
static const ClassType& GetStaticClass() { \
static ClassType s\
{\
    .Name = MakeStringID(#Class),\
    .Initializer = &Object::Create<Class>,\
    .Properties = Class::GetClassProperties(),\
    .Parent = MakeStringID(#ParentClass)\
};\
return s;\
}\
virtual const ClassType& GetStaticClassFromThis() override { if (scriptInstance.has_value()) {return *scriptInstance->ScriptClass;} return GetStaticClass(); } \
[[maybe_unused]] inline static AutoClassRegister AutoRegistrationObject_##Class = AutoClassRegister(GetStaticClass()); \
[[maybe_unused]] inline static AutoMethodRegister<ThisClass> AutoMethodRegistrationObject_##Class = AutoMethodRegister<ThisClass>();



#define CRAB_ABSTRACT_CLASS(Class, ParentClass) \
public:\
constexpr static inline std::string_view ClassName = #Class; \
using ThisClass = Class; \
using Super = ParentClass; \
static const ClassType& GetStaticClass() { \
static ClassType s\
{\
.Name = MakeStringID(#Class),\
.Initializer = nullptr,\
.Properties = Class::GetClassProperties(),\
.Parent = MakeStringID(#ParentClass),\
.Flags = ClassFlags::Abstract\
};\
return s;\
}\
virtual const ClassType& GetStaticClassFromThis() override { return GetStaticClass(); } \
[[maybe_unused]] inline static AutoClassRegister AutoRegistrationObject_##Class = AutoClassRegister(GetStaticClass());


#define CLASS_FLAG(Flag)\
[[maybe_unused]] inline static AutoClassFlagRegister<ThisClass> AutoFlagRegistrationObject_##Flag = AutoClassFlagRegister<ThisClass>(ClassFlags::##Flag);

#define REGISTER_RESOURCE_IMPORTER(Type)\
[[maybe_unused]] inline static AutoRegisterResourceImporter<Type> ImporterRegistrationObject_##Type = AutoRegisterResourceImporter<Type>();


#define BIND_METHOD(Name) \
    RegisterMethod<ThisClass>(#Name, &ThisClass::Name);