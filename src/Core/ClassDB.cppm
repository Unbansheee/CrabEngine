module;

#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <typeindex>

export module class_db;
import reflection;
import reflection_concepts;
import object;
import class_type;

export class ClassDB {
    friend struct ClassType;
    using TypeInfoRef = std::type_index;
    std::unordered_map<std::type_index, ClassType> classTypes;

public:
    template<typename T>
    const ClassType& GetClass()
    {
        return classTypes.at(std::type_index(typeid(T)));
    }

    const ClassType* GetClassByName(const std::string& name)
    {
        for (auto& type : GetClasses())
        {
            if (type.Name == name) return &classTypes.at(type.ClassIndex);
        }

        return nullptr;
    }
    
    template <typename T, typename ParentClass>
    void RegisterClass(const std::string& Name) {
        ClassType& t = classTypes[typeid(T)];
        t.ClassIndex = std::type_index(typeid(T));
        t.Name = Name;
        t.Initializer = &Object::Create<T>;
        t.Properties = T::GetClassProperties();
        t.ParentClass = std::type_index(typeid(ParentClass));
        
        // Add this class to its parents list of children
        if (t.ParentClass != typeid(Object))
        {
            classTypes[t.ParentClass].ChildClasses.push_back(t.ClassIndex);
        }
        
        std::cout << "Registered Class: " << typeid(T).name() << ". Members: ";
        for (const Property& prop : t.Properties)
        {
            std::cout << prop.name << "\n";
        }
    }

    template <typename T>
    void AddClassFlag(uint32_t flags)
    {
        ClassType& t = classTypes[typeid(T)];
        t.Flags |= flags;
    }
    
    template <typename T>
    const std::vector<Property>& GetProperties() {
        return classTypes.at(std::type_index(typeid(T))).Properties;
    }

    template <typename T>
    const std::vector<Property>& GetPropertiesOf(T* object) {
        return classTypes.at(std::type_index(typeid(T))).Properties;
    }

    template<typename T>
    const ClassType& GetParentClass()
    {
        assert(classTypes.contains(typeid(T)));
        auto t = classTypes.at(typeid(T));
        return classTypes.at(t.ParentClass);
    }

    template<typename  ParentClass, typename ChildClass>
    bool IsSubclassOf(const ChildClass& c)
    {
        if (!classTypes.contains(typeid(c))) return false;
        auto& current = classTypes.at(typeid(c));
        if (current.ClassIndex == typeid(ParentClass)) return true;
        while (current.ParentClass != typeid(ParentClass))
        {
            if (current.ParentClass == typeid(Object) && typeid(ParentClass) != typeid(Object)) return false;
            current = classTypes.at(current.ParentClass);
        }
        return true;
    }

    bool IsSubclassOf(const Object& parent, const Object& child)
    {
        if (!classTypes.contains(typeid(child))) return false;
        auto& current = classTypes.at(typeid(child));
        if (current.ClassIndex == typeid(parent)) return true;
        while (current.ParentClass != typeid(parent))
        {
            if (current.ParentClass == typeid(Object) && typeid(parent) != typeid(Object)) return false;
            current = classTypes.at(current.ParentClass);
        }
        return true;
    }

    template<typename T>
    std::vector<ClassType> GetSubclassesOf()
    {
        std::vector<ClassType> types;
        std::vector<std::type_index> stack;
        stack.push_back(typeid(T));
        while (!stack.empty())
        {
            auto curr = stack.back();
            stack.pop_back();
            auto type = classTypes.at(curr);
            types.push_back(type);

            for (auto child : type.ChildClasses)
            {
                stack.push_back(child);
            }
        }

        return types;
    }
    
    std::vector<ClassType> GetClasses();
    
    static ClassDB& Get() {
        static ClassDB instance;
        return instance;
    }
};


