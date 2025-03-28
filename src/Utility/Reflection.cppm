module;

#pragma once

#include <functional>
#include <json.hpp>
#include <string>
#include <typeindex>
#include <variant>

//#include "ReflectionMacros.h"
//#include "Resource/ResourceHandle.h"

export module Engine.Reflection;
import Engine.Variant;
import Engine.Object.Ref;
//import class_db;
//import resource;
//import class_type;


export class Property;
export class Node;

export namespace PropertyFlags
{
    constexpr uint32_t None = 0 << 0;
    constexpr uint32_t Transient = 1 << 1;
    constexpr uint32_t HideFromInspector = 1 << 2;
    constexpr uint32_t MaterialProperty = 1 << 3;
    constexpr uint32_t ColorHint = 1 << 4;
};

class Property {
public:
    // Add optional callback type
    using PostSetCallback = std::function<void(void* obj, Property& prop)>;

    template <typename T, typename Class>
    Property(const std::string& name,
        const std::string& display_name,
        const std::string& owner_class,
        T Class::*member_ptr,
        uint32_t flags,
        PostSetCallback post_set_cb = nullptr
    ) : name(name),
        displayName(display_name),
        ownerClass(owner_class),
        type(typeid(T)),
        flags(flags),
        post_set_callback(std::move(post_set_cb))
    {

        // Setter with callback invocation
        setter = [member_ptr](void* obj, const ValueVariant& value) {
            Class* target = static_cast<Class*>(obj);
            if constexpr (std::is_base_of_v<_ObjectRefBase, T>) {
                target->*member_ptr = std::get<ObjectRef<Object>>(value).Cast<T>();
            } else {
                target->*member_ptr = std::get<T>(value);
            }
        };

        getter = ([member_ptr](void* obj) -> ValueVariant {
            if constexpr (std::is_base_of_v<_ObjectRefBase, T>) {
                return ObjectRef<Object>(static_cast<Class*>(obj)->*member_ptr);
            }
            else
            {
                return static_cast<Class*>(obj)->*member_ptr;
            }
        });
    }


    Property(
    const std::string& name,
    const std::string& display_name,
    const std::string& owner_class,
    std::function<ValueVariant(void*)> getter,
    std::function<void(void*, const ValueVariant&)> setter,
    PostSetCallback postSetCallback,
    std::type_index type,
    uint32_t flags
) : name(name),
    displayName(display_name),
    ownerClass(owner_class),
    type(type),
    flags(flags),
    getter(std::move(getter)),
    setter(std::move(setter)),
    post_set_callback(std::move(postSetCallback)){}

    // Type checking interface
    template <typename T>
    bool is() const {
        return type == typeid(T);
    }

    // Safe value access
    template <typename T>
    T get(void* obj) const {
        if (!is<T>()) throw std::bad_variant_access{};
        return std::get<T>(getter(obj));
    }

    ValueVariant getVariant(void* obj) const {
        return getter(obj);
    }

    void setVariant(void* obj, ValueVariant value) const {
        setter(obj, value);
        if (post_set_callback) {
            post_set_callback(obj, *const_cast<Property*>(this));
        }
    }


    template <typename T>
    void set(void* obj, T value) const
    {
        if (!is<T>()) throw std::bad_variant_access{};
        setter(obj, value);
        // this must be const to make PropertyView's property field readonly
        // TODO: use a getter instead for propertyview
        if (post_set_callback) {
            post_set_callback(obj, *const_cast<Property*>(this));
        }
    }

    template <typename Visitor, typename ObjectClass>
    void visit(Visitor&& vis, ObjectClass* obj) const;

    template <typename Visitor, typename ObjectClass>
    void visit(Visitor&& vis, nlohmann::json& archive, ObjectClass* obj) const;

    std::string displayName;
    std::string name;
    std::string ownerClass;
    std::type_index type;
    uint32_t flags;

private:
    std::function<ValueVariant(void*)> getter;
    std::function<void(void*, const ValueVariant&)> setter;
    PostSetCallback post_set_callback;
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
export struct ClassType;

export struct PropertyView {
    const Property& property;  // Reference to property metadata
    ValueVariant& value;  // Reference to the actual value
    void* object;
    const ClassType* objectClass = nullptr;

    template <typename T>
    T& get() { return std::get<T>(value); }

    template<typename T>
    void set(T& val) { property.set(object, val); }

    const std::string& name() const { return property.name; }
    const std::string& displayName() const {return property.displayName;}
    uint32_t flags() const { return property.flags; }
};



template <typename Visitor, typename ObjectClass>
void Property::visit(Visitor&& vis, ObjectClass* obj) const
{
    auto value = getVariant(obj);
    PropertyView v = {.property= *this, .value= value, .object= obj};

    if constexpr (std::is_base_of_v<Object, ObjectClass>) {
        auto o = static_cast<Object*>(obj);
        v.objectClass = &o->GetStaticClassFromThis();
    }

    std::variant<PropertyView> p = v;
    return std::visit(std::forward<Visitor>(vis), p, value);
}

template <typename Visitor, typename ObjectClass>
void Property::visit(Visitor&& vis, nlohmann::json& archive, ObjectClass* obj) const
{
    auto value = getVariant(obj);
    PropertyView v = {.property= *this, .value= value, .object= obj};

    if constexpr (std::is_base_of_v<Object, ObjectClass>) {
        auto o = static_cast<Object*>(obj);
        v.objectClass = &o->GetStaticClassFromThis();
    }

    std::variant<PropertyView> p = v;
    std::variant<nlohmann::json*> j = &archive;
    return std::visit(std::forward<Visitor>(vis), p, j, value);
}
