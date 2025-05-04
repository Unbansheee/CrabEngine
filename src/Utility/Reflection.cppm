module;

#pragma once

#include <json.hpp>

export module Engine.Reflection;
import Engine.Variant;
import Engine.Object.Ref;
import std;

export class Property;
export struct ClassType;

export namespace PropertyFlags
{
    constexpr uint32_t None = 0 << 0;
    constexpr uint32_t Transient = 1 << 1;
    constexpr uint32_t HideFromInspector = 1 << 2;
    constexpr uint32_t MaterialProperty = 1 << 3;
    constexpr uint32_t ColorHint = 1 << 4;
    constexpr uint32_t ScriptProperty = 1 << 5;
};

template <typename>
struct is_shared_ptr : std::false_type {};

template <typename U>
struct is_shared_ptr<std::shared_ptr<U>> : std::true_type {};

template <typename T>
inline constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

template <typename T>
concept IsSharedPtrToResource =
    is_shared_ptr_v<T> &&
    std::is_base_of_v<Resource, typename T::element_type>;

template <typename T>
concept IsObjectRef =
    std::is_base_of_v<_ObjectRefBase, T> &&
    std::is_base_of_v<Object, typename T::element_type>;


// Represents a variable of a class with a getter and setter
// When provided with an Object, can set/get the variable on the object
// This drives the reflection system that handles serialization/deserialization and editor rendering
// Use the reflection macros to create these in ReflectionMacros.h
class Property {
public:
    // Optional callback type
    using PostSetCallback = std::function<void(void* obj, Property& prop)>;

    // Template version for a return type and Object type
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
        flags(flags),
        post_set_callback(std::move(post_set_cb))
    {
        if constexpr (IsSharedPtrToResource<T>) {
            reflectedObjectType = &(typename T::element_type::GetStaticClass());
        }
        else if constexpr (IsObjectRef<T>) {
            reflectedObjectType = &(typename T::element_type::GetStaticClass());
        }

        ///////////////////////////////////
        // Setter with callback invocation
        ///////////////////////////////////
        setter = [member_ptr](void* obj, const ValueVariant& value) {
            Class* target = static_cast<Class*>(obj);
            if constexpr (std::is_base_of_v<_ObjectRefBase, T>) {
                target->*member_ptr = std::get<ObjectRef<Object>>(value).Cast<T>();
            }
            else if constexpr (IsSharedPtrToResource<T>) {
                    auto base_ptr = std::get<std::shared_ptr<Resource>>(value);
                    if (base_ptr == nullptr) {
                        target->*member_ptr = nullptr;
                    }
                    else
                    {
                        // TODO: May be able to use ClassTypes here
                        auto derived_ptr = std::dynamic_pointer_cast<typename T::element_type>(base_ptr);
                        if (!derived_ptr) {
                            throw std::runtime_error("Type mismatch when assigning resource.");
                        }
                        target->*member_ptr = derived_ptr;
                    }
            }
            else {
                target->*member_ptr = std::get<T>(value);
            }
        };

        /////////////////////////////
        /// Getter
        /////////////////////////////
        getter = ([member_ptr](void* obj) -> ValueVariant {
            if constexpr (std::is_base_of_v<_ObjectRefBase, T>) {
                return ObjectRef<Object>(static_cast<Class*>(obj)->*member_ptr);
            }
            else if constexpr (IsSharedPtrToResource<T>) {
                return static_cast<std::shared_ptr<Resource>>(static_cast<Class*>(obj)->*member_ptr);
            }
            else
            {
                return static_cast<Class*>(obj)->*member_ptr;
            }
        });
    }

    // Non-templated with a custom getter/setter
    // Used for C# class properties
    Property(
    const std::string& name,
    const std::string& display_name,
    const std::string& owner_class,
    std::function<ValueVariant(void*)> getter,
    std::function<void(void*, const ValueVariant&)> setter,
    PostSetCallback postSetCallback,
    uint32_t flags
) : name(name),
    displayName(display_name),
    ownerClass(owner_class),
    flags(flags),
    getter(std::move(getter)),
    setter(std::move(setter)),
    post_set_callback(std::move(postSetCallback)) {
    }

    // Get the property value from an object
    template <typename T>
    T get(void* obj) const {
        return std::get<T>(getter(obj));
    }

    // Get the property value from an object as a generic ValueVariant
    ValueVariant getVariant(void* obj) const {
        return getter(obj);
    }

    // Set the property value on an object
    void setVariant(void* obj, ValueVariant value) const {
        setter(obj, value);
        if (post_set_callback) {
            post_set_callback(obj, *const_cast<Property*>(this));
        }
    }

    // Set the property value on an object
    template <typename T>
    void set(void* obj, T value) const
    {
        setter(obj, value);
        // this must be const to make PropertyView's property field readonly
        // TODO: use a getter instead for propertyview
        if (post_set_callback) {
            post_set_callback(obj, *const_cast<Property*>(this));
        }
    }

    // TODO: replace these
    // Visitor implementations for serialization / editor
    template <typename Visitor, typename ObjectClass>
    void visit(Visitor&& vis, ObjectClass* obj) const;

    template <typename Visitor, typename ObjectClass>
    void visit(Visitor&& vis, nlohmann::json& archive, ObjectClass* obj) const;


    std::string displayName{};
    std::string name{};
    std::string ownerClass{};
    uint32_t flags{};
    const ClassType* reflectedObjectType{};

private:
    std::function<ValueVariant(void*)> getter;
    std::function<void(void*, const ValueVariant&)> setter;
    PostSetCallback post_set_callback;
};

// PropertyView represents a property AND an object the properties are accessed from
export struct PropertyView {
    const Property& property;  // Reference to property metadata
    ValueVariant& value;  // Reference to the actual value
    void* object;
    const ClassType* objectClass = nullptr;

    // Get Proeprty value
    template <typename T>
    T& get() { return std::get<T>(value); }

    // Set property value
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

