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

export using ObjectFlags_ = uint64_t;
export namespace ObjectFlags {
    constexpr uint64_t None = 0 << 0;
    constexpr uint64_t Transient = 1 << 1; // Does not get serialized, does not get duplicated when parent node gets duplicated
    constexpr uint64_t HiddenFromTree = 1 << 2; // Does not appear in the SceneTree view
}

export class Object : public observable_dtor
{
    friend class ScriptModule;
private:
    inline static BaseObjectRegistrationObject ObjectRegistrationObject;

    friend class Property;

    ObjectFlags_ ObjectFlags = 0;
    using ThisClass = Object;
public:
    template<typename T>
    static Object* Create()
    {
        return new T();
    }
    
    ~Object() override;

    void AddFlag(ObjectFlags_ Flag);
    bool HasFlag(ObjectFlags_ Flag);

    ScriptInstance* GetScriptInstance() {
        if (scriptInstance) return scriptInstance.get();
        return nullptr;
    }

    static void RegisterMethods() {
    }

    // Returns the class properties from an object instance. This indirection is required otherwise if getting properties for a
    // Node3D via a Node*, only the Node*'s properties would be returned. This function is implemented in the BEGIN_PROPERTIES macro
    virtual const std::vector<Property>& GetPropertiesFromThis() {
        if (scriptInstance) {
            return scriptInstance->ScriptClass->Properties;
        }
        return GetClassProperties();
    }

    virtual const ClassType& GetStaticClassFromThis() {
        if (scriptInstance) {
            return *scriptInstance->ScriptClass;
        }
        return GetStaticClass();
    }

    // Returns all registered properties for this class
    static const std::vector<Property>& GetClassProperties()
    {
        static const std::vector<Property> p;
        return p;
    }

    static const ClassType& GetStaticClass();

    // Returns all registered properties for a specified class
    template <typename T>
    static const std::vector<Property>& GetPropertiesFromType() {
        return T::GetClassProperties();
    }

    // Returns all registered properties for a specified class
    template <typename T>
    static const ClassType& GetStaticClassFromType() {
        return T::GetStaticClass();
    }

    const UID& GetID() const;
    
    virtual void Serialize(nlohmann::json& archive);
    virtual void Deserialize(nlohmann::json& archive);

    virtual void OnPropertySet(Property& prop);

    template<typename T>
    static T* Cast(Object* object)
    {
        if (object->IsA(T::GetStaticClass()))
        {
            return static_cast<T*>(object);
        }
        return nullptr;
    }

    void InvalidateScriptInstance();
    void ReloadScriptInstance();

    bool IsA(const ClassType& type);
    template<typename T>
    bool IsA()
    {
        return IsA(T::GetStaticClass());
    }
    


protected:
    friend class ScriptEngine;

    UID id;
    std::unique_ptr<ScriptInstance> scriptInstance;
    std::string scriptTypeName;

    static void StaticOnPropertySet(void* obj, Property& prop) {
        auto o = static_cast<Object*>(obj);
        o->OnPropertySet(prop);
    };
};


