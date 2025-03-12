#pragma once

#include <vector>
#include "Reflection.h"
#include "json.hpp"
#include "UID.h"

class Object;
class Node;

class Object
{
public:
    template<typename T>
    static Object* Create()
    {
        return new T();
    }
    
    virtual ~Object() = default;

    // Returns the class properties from an object instance. This indirection is required otherwise if getting properties for a
    // Node3D via a Node*, only the Node*'s properties would be returned. This function is implemented in the BEGIN_PROPERTIES macro
    virtual const std::vector<Property>& GetPropertiesFromThis() { return GetClassProperties(); }

    // Returns all registered properties for this class
    static const std::vector<Property>& GetClassProperties()
    {
        static const std::vector<Property> p = {};
        return p;
    }

    // Returns all registered properties for a specified class
    template <Reflectable T>
    static const std::vector<Property>& GetPropertiesFromType() {
        return T::GetClassProperties();
    }

    const UID& GetID() const;
    
    virtual void Serialize(nlohmann::json& archive);
    virtual void Deserialize(nlohmann::json& archive);

protected:
    UID id ;
};
