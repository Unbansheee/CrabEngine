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

    static void RegisterMethods() {
    }

    // Returns the class properties from an object instance. This indirection is required otherwise if getting properties for a
    // Node3D via a Node*, only the Node*'s properties would be returned. This function is implemented in the BEGIN_PROPERTIES macro
    virtual const std::vector<Property>& GetPropertiesFromThis() { return GetClassProperties(); }

    virtual const ClassType& GetStaticClassFromThis() {
        if (scriptInstance.has_value()) {
            return *scriptInstance->ScriptClass;
        }
        return GetStaticClass();
    }

    // Returns all registered properties for this class
    static const std::vector<Property>& GetClassProperties()
    {
        static const std::vector<Property> p = {};
        return p;
    }

    template<typename T = ThisClass, typename... Args>
    static void RegisterMethod(const std::string& name, void (T::*method)(Args...)) {
            const_cast<ClassType&>(GetStaticClass()).methodTable[name] = [method](void* ctx, void* rawArgs) {
                auto* obj = static_cast<T*>(ctx);
                std::apply([&](Args... args) {
                    (obj->*method)(args...);
                }, *static_cast<std::tuple<Args...>*>(rawArgs));
            };
        }

    static const ClassType& GetStaticClass();

    // Returns all registered properties for a specified class
    template <typename T>
    static const std::vector<Property>& GetPropertiesFromType() {
        return T::GetClassProperties();
    }

    // Returns all registered properties for a specified class
    template <typename T>
    static const std::vector<Property>& GetStaticClassFromType() {
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
    

    bool IsA(const ClassType& type);
    template<typename T>
    bool IsA()
    {
        return IsA(T::GetStaticClass());
    }
    


protected:
    friend class ScriptEngine;

    UID id;
    std::optional<ScriptInstance> scriptInstance;

    static void StaticOnPropertySet(void* obj, Property& prop) {
        auto o = static_cast<Object*>(obj);
        o->OnPropertySet(prop);
    };
};


