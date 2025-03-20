#pragma once
#include <vector>
#include "typeindex"
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

#define ADD_PROPERTY_FLAGS(DisplayName, Member, Flags) \
custom.emplace_back(Property( \
#Member, \
DisplayName, \
&ThisClass::Member, \
Flags \
));

#define ADD_PROPERTY(DisplayName, Member) \
custom.emplace_back(Property( \
#Member, \
DisplayName, \
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
virtual const ClassType& GetStaticClassFromThis() override { return GetStaticClass(); } \
[[maybe_unused]] inline static AutoClassRegister AutoRegistrationObject_##Class = AutoClassRegister(GetStaticClass());


#define CRAB_ABSTRACT_CLASS(Class, ParentClass) \
public:\
using ThisClass = Class; \
using Super = ParentClass; \
static const ClassType& GetStaticClass() { \
static ClassType s\
{\
.Name = MakeStringID(#Class),\
.Initializer = nullptr,\
.Properties = Class::GetClassProperties(),\
.Parent = MakeStringID(#ParentClass),\
.Flags = (uint32_t)ClassType::ClassFlags::Abstract\
};\
return s;\
}\
virtual const ClassType& GetStaticClassFromThis() override { return GetStaticClass(); } \
[[maybe_unused]] inline static AutoClassRegister AutoRegistrationObject_##Class = AutoClassRegister(GetStaticClass());


#define CLASS_FLAG(Flag)\
[[maybe_unused]] inline static AutoClassFlagRegister<ThisClass> AutoFlagRegistrationObject_##Flag = AutoClassFlagRegister<ThisClass>((uint32_t)ClassType::ClassFlags::##Flag);

#define REGISTER_RESOURCE_IMPORTER(Type)\
[[maybe_unused]] inline static AutoRegisterResourceImporter<Type> ImporterRegistrationObject_##Type = AutoRegisterResourceImporter<Type>();