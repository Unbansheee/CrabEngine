#pragma once
#include <functional>
#include <iostream>

#include "Nodes/Node.h"
#include "Object.h"

struct ClassType
{
    using CreateClassFn = std::function<Object*()>;
    std::string Name;
    CreateClassFn Initializer;
    std::vector<Property> Properties;
};

class ClassDB {
    using TypeInfoRef = std::type_index;
    std::unordered_map<std::type_index, ClassType> classTypes;

public:
    template <Reflectable T>
    void RegisterClass(const std::string& Name) {
        ClassType t;
        t.Name = Name;
        t.Initializer = &Object::Create<T>;
        t.Properties = T::GetClassProperties();
        classTypes[std::type_index(typeid(T))] = t;
        
        std::cout << "Registered Class: " << typeid(T).name() << ". Members: ";
        for (const Property& prop : t.Properties)
        {
            std::cout << prop.name << "\n";
        }
    }

    template <Reflectable T>
    const std::vector<Property>& GetProperties() {
        return classTypes[std::type_index(typeid(T))].Properties;
    }

    template <Reflectable T>
    const std::vector<Property>& GetPropertiesOf(T* object) {
        return classTypes[std::type_index(typeid(T))].Properties;
    }

    std::vector<ClassType> GetClasses();
    
    static ClassDB& Get() {
        static ClassDB instance;
        return instance;
    }
};

// Auto-registration helper
template <Reflectable T>
struct AutoRegister {
    AutoRegister(const std::string& Name) {
        ClassDB::Get().RegisterClass<T>(Name);
    }
};

// Usage: Place this in .cpp files
#define REGISTER_CLASS(Class) \
AutoRegister<Class> AutoRegistrationObject_##Class(#Class);

