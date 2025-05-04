module;

#pragma once

#include <vector>

export module Engine.Object;
export import Engine.Reflection.Class;
export import Engine.UID;
export import json;
export import std;
import Engine.Object.ObservableDtor;
import Engine.ScriptInstance;
import Engine.Reflection;

struct BaseObjectRegistrationObject
{
    BaseObjectRegistrationObject();
};

// Object instance flags
export using ObjectFlags_ = uint64_t;
export namespace ObjectFlags {
    constexpr uint64_t None = 0 << 0;
    constexpr uint64_t Transient = 1 << 1; // Does not get serialized, does not get duplicated when parent node gets duplicated
    constexpr uint64_t HiddenFromTree = 1 << 2; // Does not appear in the SceneTree view
}

export class Object : public observable_dtor
{
    friend class ScriptModule;
    friend class ScriptEngine;
    friend class Property;
    using ThisClass = Object;

public:
    // I don't particularly remember why this is the convention. Possibly for future use
    template<typename T>
    static Object* Create()
    {
        return new T();
    }
    
    ~Object() override;

    void AddFlag(ObjectFlags_ Flag);
    bool HasFlag(ObjectFlags_ Flag);

    // Get the unique ID of this object
    const UID& GetID() const;

    // Get the C# managed script instance if one exists
    ScriptInstance* GetScriptInstance();

    // Output this object's properties to JSON
    virtual void Serialize(nlohmann::json& archive);
    // Populate this object's properties from json
    virtual void Deserialize(nlohmann::json& archive);

    // Called when an objects property is set via property.set()
    virtual void OnPropertySet(Property& prop);

    ////////////////////
    // Property Reflection
    ////////////////////
    // Returns the class properties from an object instance. This indirection is required otherwise if getting properties for a
    // Node3D via a Node*, only the Node*'s properties would be returned. This function is implemented in the BEGIN_PROPERTIES macro
    virtual const std::vector<Property>& GetPropertiesFromThis();
    // Returns all registered properties for this class (static)
    static const std::vector<Property>& GetClassProperties()
    {
        static const std::vector<Property> p;
        return p;
    }

    // Returns all registered properties for a specified class (static)
    template <typename T>
    static const std::vector<Property>& GetPropertiesFromType() {
        return T::GetClassProperties();
    }

    ////////////////////
    // Class Reflection
    ////////////////////
    // Get the static class from an object instance
    virtual const ClassType& GetStaticClassFromThis();
    static const ClassType& GetStaticClass();
    // Returns all registered properties for a specified class
    template <typename T>
    static const ClassType& GetStaticClassFromType() {
        return T::GetStaticClass();
    }

    bool IsA(const ClassType& type);
    template<typename T> bool IsA() { return IsA(T::GetStaticClass()); }

    template<typename T>
    static T* Cast(Object* object)
    {
        if (object->IsA(T::GetStaticClass()))
        {
            return static_cast<T*>(object);
        }
        return nullptr;
    }

protected:
    UID id;

    // C# Managed script - if this exists, then this object instance's "ClassType" is dictated by this
    std::unique_ptr<ScriptInstance> scriptInstance;
    // Temp fix for hot reload script reinstantiation
    std::string scriptTypeName;

    static void StaticOnPropertySet(void* obj, Property& prop) {
        auto o = static_cast<Object*>(obj);
        o->OnPropertySet(prop);
    };

private:
    inline static BaseObjectRegistrationObject ObjectRegistrationObject;
    ObjectFlags_ ObjectFlags = 0;

    // Called only by ScriptEngine/ScriptModule
    void InvalidateScriptInstance();
    void ReloadScriptInstance();
};


