#pragma once
#include <functional>
#include <iostream>

#include "Object.h"

class ClassDB {
    using TypeInfoRef = std::type_index;
    std::unordered_map<std::type_index, std::vector<Property>> classProps;

public:
    template <Reflectable T>
    void RegisterClass() {
        const auto& props = T::GetClassProperties();
        classProps[std::type_index(typeid(T))] = props;

        std::cout << "Registered Class: " << typeid(T).name() << ". Members: ";
        for (const Property& prop : props)
        {
            std::cout << prop.name << "\n";
        }
    }

    template <Reflectable T>
    const std::vector<Property>& GetProperties() {
        return classProps[std::type_index(typeid(T))];
    }

    template <Reflectable T>
    const std::vector<Property>& GetPropertiesOf(T* object) {
        return classProps[std::type_index(typeid(T))];
    }
    
    static ClassDB& Get() {
        static ClassDB instance;
        return instance;
    }
};

// Auto-registration helper
template <Reflectable T>
struct AutoRegister {
    AutoRegister() {
        ClassDB::Get().RegisterClass<T>();
    }
};

// Usage: Place this in .cpp files
#define REGISTER_CLASS(Class) \
AutoRegister<Class> AutoRegistrationObject_##Class;

