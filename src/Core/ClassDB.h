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
};

class ClassDB {
    using TypeInfoRef = std::type_index;
    std::unordered_map<std::type_index, std::vector<Property>> classProps;
    std::unordered_map<std::type_index, ClassType> classTypes;

public:
    template <Reflectable T>
    void RegisterClass(const std::string& Name) {
        const auto& props = T::GetClassProperties();
        classProps[std::type_index(typeid(T))] = props;

        std::cout << "Registered Class: " << typeid(T).name() << ". Members: ";
        for (const Property& prop : props)
        {
            std::cout << prop.name << "\n";
        }

        ClassType t;
        t.Name = Name;
        t.Initializer = &Object::Create<T>;
        classTypes[std::type_index(typeid(T))] = t;
    }

    template <Reflectable T>
    const std::vector<Property>& GetProperties() {
        return classProps[std::type_index(typeid(T))];
    }

    template <Reflectable T>
    const std::vector<Property>& GetPropertiesOf(T* object) {
        return classProps[std::type_index(typeid(T))];
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

