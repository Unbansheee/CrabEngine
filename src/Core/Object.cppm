module;

#pragma once

#include <vector>

export module Engine.Object;
export import Engine.Reflection;
export import uid;
export import json;
export import std;
import Engine.Object.ObservableDtor;
export struct ClassType;

struct BaseObjectRegistrationObject
{
    BaseObjectRegistrationObject();
};


export class Object : public IPropertyInterface, public observable_dtor
{
private:
    inline static BaseObjectRegistrationObject ObjectRegistrationObject;
public:
    template<typename T>
    static Object* Create()
    {
        return new T();
    }
    
    ~Object() override = default;

    // Returns the class properties from an object instance. This indirection is required otherwise if getting properties for a
    // Node3D via a Node*, only the Node*'s properties would be returned. This function is implemented in the BEGIN_PROPERTIES macro
    virtual const std::vector<Property>& GetPropertiesFromThis() { return GetClassProperties(); }
    virtual const ClassType& GetStaticClassFromThis() { return GetStaticClass(); }

    // Returns all registered properties for this class
    static const std::vector<Property>& GetClassProperties()
    {
        static const std::vector<Property> p = {};
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
    static const std::vector<Property>& GetStaticClassFromType() {
        return T::GetStaticClass();
    }

    const UID& GetID() const;
    
    virtual void Serialize(nlohmann::json& archive);
    virtual void Deserialize(nlohmann::json& archive);

    virtual void OnPropertySet(Property& prop) override;



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
    UID id ;
};


