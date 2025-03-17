module;
#include <cstdint>
#include <functional>
#include <typeindex>

export module class_type;
export import string_id;

export class Object;
export class Property;



export class BAD_OBJECT
{
};

struct NoCopy{
    // having the defaulted constructor is required for C++20 aggregate initialization to work
    NoCopy() = default;
    // prevents copy and deletes other constructors
    NoCopy(const NoCopy&) = delete;
};

export struct ClassType : NoCopy
{
    enum class ClassFlags : uint32_t
    {
        None = 0 << 0,
        EditorVisible = 1 << 1,
        Abstract = 1 << 2
    };
    
    using CreateClassFn = std::function<Object*()>;
    string_id Name = MakeStringID("null");
    CreateClassFn Initializer;
    std::vector<Property> Properties;
    string_id Parent = MakeStringID("null");
    uint32_t Flags;

    bool operator==(const ClassType& other) const
    {
        return other.Name == Name;
    }

    operator bool() const
    {
        return IsValid();
    }

    bool IsValid() const
    {
        return Name != MakeStringID("null");
    }
    
    
    bool IsSubclassOf(const ClassType& parent) const;

    template<typename T>
    bool IsSubclassOf()
    {
        return IsSubclassOf(T::GetStaticClass());
    }

    void AddFlag(uint32_t Flag)
    {
        Flags |= Flag;
    };

    bool HasFlag(uint32_t Flag) const
    {
        return Flags & Flag;
    }
};
