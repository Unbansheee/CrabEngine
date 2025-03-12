#pragma once
#include <functional>
#include <iostream>

//#include "Nodes/Node.h"
#include "Object.h"

enum class ClassFlags : uint32_t
{
    EditorVisible = 1 << 0
};

struct ClassType
{
    using CreateClassFn = std::function<Object*()>;
    std::type_index ClassIndex = std::type_index(typeid(Object));
    std::string Name;
    CreateClassFn Initializer;
    std::vector<Property> Properties;
    std::type_index ParentClass = std::type_index(typeid(Object));
    std::vector<std::type_index> ChildClasses;
    uint32_t Flags;
};

class ClassDB {
    using TypeInfoRef = std::type_index;
    std::unordered_map<std::type_index, ClassType> classTypes;

public:
    template<Reflectable T>
    const ClassType& GetClass()
    {
        return classTypes.at(std::type_index(typeid(T)));
    }

    template<Reflectable T>
    const ClassType& GetClass(T& object)
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

    
    
    
    template <Reflectable T, Reflectable ParentClass>
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

    template <Reflectable T>
    void AddClassFlag(uint32_t flags)
    {
        ClassType& t = classTypes[typeid(T)];
        t.Flags |= flags;
    }
    
    template <Reflectable T>
    const std::vector<Property>& GetProperties() {
        return classTypes.at(std::type_index(typeid(T))).Properties;
    }

    template <Reflectable T>
    const std::vector<Property>& GetPropertiesOf(T* object) {
        return classTypes.at(std::type_index(typeid(T))).Properties;
    }

    template<Reflectable T>
    const ClassType& GetParentClass()
    {
        assert(classTypes.contains(typeid(T)));
        auto t = classTypes.at(typeid(T));
        return classTypes.at(t.ParentClass);
    }

    template<Reflectable ParentClass, Reflectable ChildClass>
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

    template<Reflectable T>
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

// Auto-registration helper
template <Reflectable T, Reflectable ParentClass>
struct AutoRegister {
    AutoRegister(const std::string& Name) {
        ClassDB::Get().RegisterClass<T, ParentClass>(Name);
    }
};

// Auto-registration helper
template <Reflectable T>
struct AutoClassFlagRegister {
    AutoClassFlagRegister(uint32_t flags) {
        ClassDB::Get().AddClassFlag<T>(flags);
    }
};

// Usage: Place this in .cpp files
#define CRAB_CLASS(Class, ParentClass) \
    public:\
    using ThisClass = Class; \
    using Super = ParentClass; \
    static const ClassType& GetStaticClass() { return ClassDB::Get().GetClass<ThisClass>(); }\
    virtual const ClassType& GetStaticClassFromThis() override { return GetStaticClass(); } \
    static inline AutoRegister<Class, ParentClass> AutoRegistrationObject_##Class = AutoRegister<Class, ParentClass>(#Class);

#define CLASS_FLAG(Flag)\
    static inline AutoClassFlagRegister<ThisClass> AutoFlagRegistrationObject_##Flag = AutoClassFlagRegister<ThisClass>((uint32_t)ClassFlags::##Flag);
