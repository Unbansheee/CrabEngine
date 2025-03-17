module;

#pragma once

#include <functional>
#include <json.hpp>
#include <string>
#include <typeindex>
#include <variant>

//#include "ReflectionMacros.h"
//#include "Resource/ResourceHandle.h"

export module reflection;
import variant;
//import class_db;
//import resource;
//import class_type;



export class IPropertyInterface;

export class Property {
public:
    enum class Flags : uint32_t
    {
        None = 0 << 0,
        Transient = 1 << 1,
        HideFromInspector = 1 << 2,
        MaterialProperty = 1 << 3
    };

    template <typename T, typename Class>
    Property(const std::string& name, T Class::*member_ptr, Flags flags) 
        : name(name),
          type(typeid(T)),
          flags(flags),
          getter([member_ptr](IPropertyInterface* obj) -> ValueVariant {
              return static_cast<Class*>(obj)->*member_ptr;
          }),
        setter( [member_ptr](IPropertyInterface* obj, const ValueVariant& value) {
            static_cast<Class*>(obj)->*member_ptr = std::get<T>(value);
        })
    {
        
    }

    // Type checking interface
    template <typename T>
    bool is() const {
        return type == typeid(T);
    }

    // Safe value access
    template <typename T>
    T get(IPropertyInterface* obj) const {
        if (!is<T>()) throw std::bad_variant_access{};
        return std::get<T>(getter(obj));
    }

    template <typename T>
    void set(IPropertyInterface* obj, T value) const
    {
        if (!is<T>()) throw std::bad_variant_access{};
        setter(obj, value);
        // this must be const to make PropertyView's property field readonly
        // TODO: use a getter instead for propertyview
        TriggerPropertySetOn(obj);
        
    }

    template <typename Visitor>
    void visit(Visitor&& vis, IPropertyInterface* obj) const;

    template <typename Visitor>
    void visit(Visitor&& vis, nlohmann::json& archive, IPropertyInterface* obj) const;

    std::string name;
    std::type_index type;
    Flags flags;

private:
    void TriggerPropertySetOn(IPropertyInterface* obj) const;
    
    std::function<ValueVariant(IPropertyInterface*)> getter;
    std::function<void(IPropertyInterface*, const ValueVariant&)> setter;

    ValueVariant getVariant(IPropertyInterface* obj) const {
        return getter(obj); 
    }

};



/*
// Concept for Resource-derived types
export template<typename T>
concept IsResource = 
    std::is_base_of_v<Resource, T> && 
    requires {
    // Enforce reflection system requirements
    { T::StaticClass() } -> std::convertible_to<const ClassType&>;
    };
*/

export struct PropertyView {
    const Property& property;  // Reference to property metadata
    ValueVariant& value;  // Reference to the actual value
    IPropertyInterface* object;

    template <typename T>
    T& get() { return std::get<T>(value); }

    template<typename T>
    void set(T& val) { property.set(object, val); }

    const std::string& name() const { return property.name; }
    Property::Flags flags() const { return property.flags; }
};



template <typename Visitor>
void Property::visit(Visitor&& vis, IPropertyInterface* obj) const
{
    auto value = getVariant(obj);
    PropertyView v = {.property= *this, .value= value, .object= obj};
    std::variant<PropertyView> p = v;
    return std::visit(std::forward<Visitor>(vis), p, value);
}

template <typename Visitor>
void Property::visit(Visitor&& vis, nlohmann::json& archive, IPropertyInterface* obj) const
{
    auto value = getVariant(obj);
    PropertyView v = {.property= *this, .value= value, .object= obj};
    std::variant<PropertyView> p = v;
    std::variant<nlohmann::json*> j = &archive;
    return std::visit(std::forward<Visitor>(vis), p, j, value);
}

class IPropertyInterface
{
    friend class Property;
protected:
    virtual void OnPropertySet(Property& prop) = 0;
};
