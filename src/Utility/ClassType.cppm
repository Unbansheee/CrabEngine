module;
#include <cstdint>
#include <functional>
#include <string>
#include <typeindex>

export module class_type;

export class Object;
export class Property;

export enum class ClassFlags : uint32_t
{
    EditorVisible = 1 << 0
};

class UndefinedObject
{
};

export struct ClassType
{
    using CreateClassFn = std::function<Object*()>;
    std::type_index ClassIndex = std::type_index(typeid(UndefinedObject));
    std::string Name;
    CreateClassFn Initializer;
    std::vector<Property> Properties;
    std::type_index ParentClass = std::type_index(typeid(UndefinedObject));
    std::vector<std::type_index> ChildClasses;
    uint32_t Flags;

    bool operator==(const ClassType& other) const
    {
        return other.ClassIndex == ClassIndex;
    }

    static const ClassType& Get(std::type_index id);
    bool IsSubclassOf(const ClassType& parent) const;

    template<typename T>
    bool IsSubclassOf()
    {
        return IsSubclassOf(T::GetStaticClass());
    }
};
