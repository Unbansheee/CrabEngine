module;

#pragma once

#include <functional>
#include <iostream>
#include <typeindex>

export module Engine.Reflection.ClassDB;
import Engine.Reflection;
import Engine.Reflection.Concepts;
import Engine.Object;
import Engine.Reflection.Class;
import Engine.StringID;

export class ClassDB {
    friend struct ClassType;
    using TypeInfoRef = std::type_index;
    //std::unordered_map<std::type_index, ClassType> classTypes;
    std::vector<const ClassType*> allClassTypes;
    std::unordered_map<string_id, const ClassType*> classNameLookup;
    std::unordered_multimap<string_id, const ClassType*> classHierarchy;

public:
    template<typename T>
    const ClassType* GetClass()
    {
        return &T::GetStaticClass();
    }

    const ClassType* GetClassByName(const std::string& name)
    {
        string_id n = MakeStringID(name);
        for (auto& type : GetClasses())
        {
            if (type->Name == n) return type;
        }

        return nullptr;
    }

    /*
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
    */

    void RegisterClassType(const ClassType& classType)
    {
        allClassTypes.emplace_back(&classType);
        classNameLookup[classType.Name] = &classType;
        classHierarchy.insert({classType.Parent, &classType});
        
        std::cout << "Registered: " << classType.Name.string() << "\n";
    }

    template <typename T>
    void AddClassFlag(uint32_t flags)
    {
        const ClassType& classT = T::GetStaticClass();
        auto& t = const_cast<ClassType&>(classT);
        t.Flags |= flags;
    }
    
    template <typename T>
    const std::vector<Property>& GetProperties() {
        return T::GetStaticClass().Properties;
    }

    template <typename T>
    const std::vector<Property>& GetPropertiesOf(T* object) {
        return T::GetStaticClass().Properties;
    }

    template<typename T>
    const ClassType* GetParentClass()
    {
        return T::GetStaticClass().Parent;
    }

    const ClassType* GetParentClass(string_id id)
    {
        return classNameLookup.at(id);
    }
    

    template<typename T>
    std::vector<const ClassType*> GetSubclassesOf()
    {
        
        std::vector<const ClassType*> types;
        std::vector<string_id> stack;
        stack.push_back(T::GetStaticClass().Name);
        while (!stack.empty())
        {
            auto curr = stack.back();
            stack.pop_back();
            auto type = classNameLookup.at(curr);
            types.push_back(type);

            auto e = classHierarchy.find(curr);
            auto range = classHierarchy.equal_range(curr);
            for (auto it = range.first; it != range.second; ++it)
            {
                stack.push_back(it->second->Name);
            }
            
            //for (const ClassType* child : classHierarchy.find(curr))
            //{
            //    stack.push_back(child->Name);
            //}
        }

        return types;
    }
    
    const std::vector<const ClassType*>& GetClasses();
    
    static ClassDB& Get() {
        static ClassDB instance;
        return instance;
    }
};


