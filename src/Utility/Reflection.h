#pragma once
#include <functional>
#include <iostream>
#include <json.hpp>
#include <memory>
#include <string>
#include <ranges>
#include <typeindex>
#include <variant>

#include "CrabTypes.h"
#include "Transform.h"


class Property;
class PropertySupplier;
struct PropertyView;
class Object;

class Property {
public:
    // Supported native types + common engine types
    using ValueVariant = std::variant<
        int, float, bool, 
        std::string, Vector3, Vector2,
        Quat,
        Vector4, Transform
        // Add other types as needed
    >;

    enum class Flags : uint32_t
    {
        None = 0 << 0,
        Transient = 1 << 0,
        HideFromInspector = 1 << 1,
    };

    template <typename T, typename Class>
    Property(const std::string& name, T Class::*member_ptr, Flags flags = Flags::None) 
        : name(name),
          type(typeid(T)),
          flags(flags),
          getter([member_ptr](Object* obj) -> ValueVariant {
              return static_cast<Class*>(obj)->*member_ptr;
          }),
          setter([member_ptr](Object* obj, const ValueVariant& value) {
              static_cast<Class*>(obj)->*member_ptr = std::get<T>(value);
          }) {}

    // Type checking interface
    template <typename T>
    bool is() const {
        return type == typeid(T);
    }

    // Safe value access
    template <typename T>
    T get(Object* obj) const {
        if (!is<T>()) throw std::bad_variant_access{};
        return std::get<T>(getter(obj));
    }

    template <typename T>
    void set(Object* obj, T value) const {
        if (!is<T>()) throw std::bad_variant_access{};
        setter(obj, value);
    }
    
    template <typename Visitor>
    void visit(Visitor&& vis, Object* obj) const;

    template <typename Visitor>
    void visit(Visitor&& vis, nlohmann::json& archive, Object* obj) const;

    std::string name;
    std::type_index type;
    Flags flags;

private:
    std::function<ValueVariant(Object*)> getter;
    std::function<void(Object*, const ValueVariant&)> setter;

    ValueVariant getVariant(Object* obj) const {
        return getter(obj);  // Your existing getter
    }

};


// Concept for auto-detecting reflected classes
template <typename T>
concept Reflectable = requires {
    { T::GetClassProperties() } -> std::same_as<const std::vector<Property>&>;
};

struct PropertyView {
    const Property& property;  // Reference to property metadata
    Property::ValueVariant& value;  // Reference to the actual value
    Object* object;

    template <typename T>
    T& get() { return std::get<T>(value); }

    template<typename T>
    void set(T& val) { property.set(object, val); }

    const std::string& name() const { return property.name; }
    Property::Flags flags() const { return property.flags; }
};

template <typename Visitor>
void Property::visit(Visitor&& vis, Object* obj) const
{
    auto value = getVariant(obj);
    PropertyView v = {.property= *this, .value= value, .object= obj};
    std::variant<PropertyView> p = v;
    return std::visit(std::forward<Visitor>(vis), p, value);
}

template <typename Visitor>
void Property::visit(Visitor&& vis, nlohmann::json& archive, Object* obj) const
{
    auto value = getVariant(obj);
    PropertyView v = {.property= *this, .value= value, .object= obj};
    std::variant<PropertyView> p = v;
    std::variant<nlohmann::json*> j = &archive;
    return std::visit(std::forward<Visitor>(vis), p, j, value);
}


#define BEGIN_PROPERTIES(ParentType) \
virtual const std::vector<Property>& GetPropertiesFromThis() override { return GetClassProperties(); }\
static const auto& GetClassProperties() { \
static const std::vector<Property> props = []{ \
std::vector<Property> base = ParentType::GetClassProperties(); \
std::vector<Property> custom;

#define ADD_PROPERTY_FLAGS(Type, Name, Member, Flags) \
custom.push_back(Property( \
Name, \
&Type::Member, \
Flags \
));

#define ADD_PROPERTY(Type, Name, Member) \
custom.push_back(Property( \
Name, \
&Type::Member, \
Property::Flags::None \
));

#define END_PROPERTIES \
base.insert(base.end(), custom.begin(), custom.end()); \
return base; \
}(); \
return props; \
}\
