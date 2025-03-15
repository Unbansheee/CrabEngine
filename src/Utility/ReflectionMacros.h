#pragma once
#include <vector>
import reflection;
import class_db;
import class_type;
import auto_registration;

#define BEGIN_PROPERTIES \
virtual const std::vector<Property>& GetPropertiesFromThis() override { return GetClassProperties(); }\
static const auto& GetClassProperties() { \
static const std::vector<Property> props = []{ \
std::vector<Property> base = Super::GetClassProperties(); \
std::vector<Property> custom;

#define ADD_PROPERTY_FLAGS(Name, Member, Flags) \
custom.push_back(Property( \
Name, \
&ThisClass::Member, \
Flags \
));

#define ADD_PROPERTY(Name, Member) \
custom.push_back(Property( \
Name, \
&ThisClass::Member, \
Property::Flags::None \
));

#define END_PROPERTIES \
base.insert(base.end(), custom.begin(), custom.end()); \
return base; \
}(); \
return props; \
}\

// Usage: Place this in .cpp files
#define CRAB_CLASS(Class, ParentClass) \
public:\
using ThisClass = Class; \
using Super = ParentClass; \
static const ClassType& GetStaticClass() { return ClassDB::Get().GetClass<ThisClass>(); }\
virtual const ClassType& GetStaticClassFromThis() override { return GetStaticClass(); } \
[[maybe_unused]] inline static AutoRegister<Class, ParentClass> AutoRegistrationObject_##Class = AutoRegister<Class, ParentClass>(#Class);

#define CLASS_FLAG(Flag)\
[[maybe_unused]] inline static AutoClassFlagRegister<ThisClass> AutoFlagRegistrationObject_##Flag = AutoClassFlagRegister<ThisClass>((uint32_t)ClassFlags::##Flag);

#define REGISTER_RESOURCE_IMPORTER(Type)\
[[maybe_unused]] inline static AutoRegisterResourceImporter<Type> ImporterRegistrationObject_##Type = AutoRegisterResourceImporter<Type>();