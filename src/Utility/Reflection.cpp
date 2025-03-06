#include <Reflection.h>

#include "Node3D.h"
#include "imgui/imgui.h"
#include "Resource/TextureResource.h"
#include "imgui/misc/cpp/imgui_stdlib.h"


void ImGui::DrawImGuiStringControl(PropertyView& property, std::string* value)
{
    if (ImGui::InputText(property.name().c_str(), value))
    {
        property.set(*value);
    }
}

void ImGui::DrawImGuiCheckboxControl(PropertyView& property, bool* value)
{
    if (ImGui::Checkbox(property.name().c_str(), value))
    {
        property.set(*value);
    }
}

void ImGui::DrawImGuiTransformControl(PropertyView& property, Transform* value)
{
    bool dirty = false;
    ImGui::Text("Transform");
    
    if (ImGui::DragFloat3("Position", &value->Position.x, 0.1f))
    {
        dirty = true;
    }

    auto euler = glm::degrees(glm::eulerAngles(value->Orientation));
    if (ImGui::DragFloat3("Rotation", &euler.x, 0.1f))
    {
        value->Orientation = Quat(glm::radians(euler));
        dirty = true;
    }
    if (ImGui::DragFloat3("Scale", &value->Scale.x, 0.1f))
    {
        dirty = true;
    }
    
    if (dirty)
    {
        property.set(*value);
        if (auto node = dynamic_cast<Node*>(property.object))
        {
            node->UpdateTransform();
        }
    }
}

void ImGui::DrawImGuiVector3Control(PropertyView& property, Vector3* value)
{
    if (ImGui::DragFloat3(property.name().c_str(), &value->x))
    {
        property.set(*value);
    }
}

void ImGui::DrawImGuiVector2Control(PropertyView& property, Vector2* value)
{
    if (ImGui::DragFloat2(property.name().c_str(), &value->x))
    {
        property.set(*value);
    }
}

void ImGui::DrawImGuiVector4Control(PropertyView& property, Vector4* value)
{
    if (ImGui::DragFloat4(property.name().c_str(), &value->x))
    {
        property.set(*value);
    }
}

void ImGui::DrawImGuiFloatControl(PropertyView& property, float* value)
{
    if (ImGui::DragFloat(property.name().c_str(), value))
    {
        property.set(*value);
    }
}

void ImGui::DrawImGuiIntControl(PropertyView& property, int* value)
{
    if (ImGui::DragInt(property.name().c_str(), value))
    {
        property.set(*value);
    }
}
