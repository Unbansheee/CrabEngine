module;

#pragma once

export module Engine.Reflection.ClassDB;
import Engine.Reflection;
import Engine.Reflection.Concepts;
import Engine.Object;
import Engine.Reflection.Class;
import Engine.StringID;

export class ClassDB {
    friend struct ClassType;
    std::vector<const ClassType*> allClassTypes{};
    std::unordered_map<string_id, const ClassType*> classNameLookup{};
    std::unordered_multimap<string_id, const ClassType*> classHierarchy{};

    ClassDB() = default;
public:
    ClassDB(const ClassDB& other) = delete;

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

    const ClassType* GetClassByName(const string_id& name)
    {
        for (auto& type : GetClasses())
        {
            if (type->Name == name) return type;
        }

        return nullptr;
    }

    void RegisterClassType(const ClassType& classType)
    {
        allClassTypes.emplace_back(&classType);
        classNameLookup[classType.Name] = &classType;
        classHierarchy.insert({classType.Parent, &classType});
        
        std::cout << "Registered: " << classType.Name.string() << "\n";
    }

    void UnregisterClassType(const ClassType& classType)
    {
        allClassTypes.erase(std::ranges::find(allClassTypes, &classType));
        classNameLookup.erase(classType.Name);
        classHierarchy.erase(classType.Name);
        for (auto it = classHierarchy.begin(); it != classHierarchy.end(); ) {
            if (it->second == &classType)
                it = classHierarchy.erase(it);
            else
                ++it;
        }


        std::cout << "Unregistered: " << classType.Name.string() << "\n";
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
        return classNameLookup.at(T::GetStaticClass().Parent);
    }

    const ClassType* GetParentClass(const ClassType* ofClass)
    {
        return classNameLookup.at(ofClass->Parent);
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
                auto me = curr.string();
                if (!classNameLookup.contains(it->second->Name)) {
                    auto name = curr.string();
                    std::cout << "Broken class: " << name << std::endl;
                }

                stack.push_back(it->second->Name);
            }
        }

        return types;
    }

    const ClassType* GetNativeType(const ClassType* scriptType) {
        const ClassType* current = scriptType;
        while (current->HasFlag(ClassFlags::ScriptClass)) {
            current = GetParentClass(current);
        }
        return current;
    }

    const std::vector<const ClassType*>& GetClasses();
    
    static ClassDB& Get();
};


