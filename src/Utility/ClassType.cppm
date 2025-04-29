module;
#include <cstdint>
#include <functional>

export module Engine.Reflection.Class;
export import Engine.StringID;
import Engine.Reflection;

export class BAD_OBJECT
{
};

export class Object;

struct NoCopy{
    // having the defaulted constructor is required for C++20 aggregate initialization to work
    NoCopy() = default;
    // prevents copy and deletes other constructors
    NoCopy(const NoCopy&) = delete;

    NoCopy(NoCopy&&) = default;
};

using ClassFlags_ = uint32_t;
export namespace ClassFlags {
    constexpr uint32_t None = 0 << 0;
    constexpr uint32_t EditorVisible = 1 << 1;
    constexpr uint32_t Abstract = 1 << 2;
    constexpr uint32_t ScriptClass = 1 << 3;
}

export using MethodFn = std::function<void(void*, void*)>;

export struct ClassType : NoCopy
{
    using CreateClassFn = std::function<Object*()>;
    string_id Name = MakeStringID("null");
    CreateClassFn Initializer{};
    std::vector<Property> Properties{};
    string_id Parent = MakeStringID("null");
    ClassFlags_ Flags = 0;
    std::unordered_map<std::string, MethodFn> methodTable;

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
    bool IsSubclassOf() const
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
