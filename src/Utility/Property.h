#pragma once
#include <functional>
#include <memory>
#include <string>
#include <ranges>
#include <variant>

#include "CrabTypes.h"

struct Transform;

struct Property;

/*
enum class PropertyType {
    FLOAT,
    INT,
    BOOL,
    STRING,
    VEC2,
    VEC3,
    COLOR
};
*/

class PropertySupplier;

namespace ImGui
{
    void DrawImGuiStringControl(const Property& property, std::string* value);
    void DrawImGuiCheckboxControl(const Property& property, bool* value);
    void DrawImGuiTransformControl(const Property& property, Transform* value);
    void DrawImGuiVector3Control(const Property& property, Vector3* value);
    void DrawImGuiVector2Control(const Property& property, Vector2* value);
    void DrawImGuiVector4Control(const Property& property, Vector4* value);
    void DrawImGuiFloatControl(const Property& property, float* value);
    void DrawImGuiIntControl(const Property& property, int* value);

}


class PropertySupplier
{
public:
    virtual std::vector<Property> GetProperties() = 0;
};


struct Property
{
    using PropertyType = std::variant<bool*, std::string*, Transform*, Vector4*, Vector3*, Vector2*, int*, float*>;
    enum class PropertyFlags : uint8_t
    {
        Transient = 1 << 0,
        HideFromInspector = 1 << 1,
    };


    
    Property(PropertySupplier* property_owner, const std::string& property_name, PropertyType property_ref, PropertyFlags flags = PropertyFlags(0))
        : owner(property_owner), name(property_name), value(property_ref), propertyFlags(flags) {}

    PropertySupplier* owner;
    std::string name;
    PropertyType value;
    std::function<void(Property)> customDrawFunction;
    PropertyFlags propertyFlags;

    template<class... Ts>
    struct overload : Ts... {
        using Ts::operator()...;
    };

    template<class... Ts>
    overload(Ts...) -> overload<Ts...>;
    
    void DrawProperty()
    {
        if (customDrawFunction) customDrawFunction(*this);
        else
        {
            const auto visitor = overload
            {
                [this](Transform* v){ ImGui::DrawImGuiTransformControl(*this, v); },
                [this](std::string* v){ ImGui::DrawImGuiStringControl(*this, v); },
                [this](bool* v){ ImGui::DrawImGuiCheckboxControl(*this, v); },
                [this](Vector4* v){ ImGui::DrawImGuiVector4Control(*this, v); },
                [this](Vector3* v){ ImGui::DrawImGuiVector3Control(*this, v); },
                [this](Vector2* v){ ImGui::DrawImGuiVector2Control(*this, v); },
                [this](float* v){ ImGui::DrawImGuiFloatControl(*this, v); },
                [this](int* v){ ImGui::DrawImGuiIntControl(*this, v); },
            };
            
            std::visit(visitor, value);
        }
    };
};


