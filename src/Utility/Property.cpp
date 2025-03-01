#include <Property.h>

#include "imgui/imgui.h"
#include "Resource/TextureResource.h"
#include "imgui/misc/cpp/imgui_stdlib.h"


void ImGui::DrawImGuiStringControl(const Property& property, std::string* value)
{
    ImGui::InputText(property.name.c_str(), value);
}

void ImGui::DrawImGuiCheckboxControl(const Property& property, bool* value)
{
    ImGui::Checkbox(property.name.c_str(), value);
}

void ImGui::DrawImGuiTransformControl(const Property& property, Transform* value)
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
        if (auto node = dynamic_cast<Node*>(property.owner))
        {
            node->UpdateTransform();
        }
    }
}

void ImGui::DrawImGuiVector3Control(const Property& property, Vector3* value)
{
    ImGui::DragFloat3(property.name.c_str(), &value->x);
}

void ImGui::DrawImGuiVector2Control(const Property& property, Vector2* value)
{
    ImGui::DragFloat2(property.name.c_str(), &value->x);
}

void ImGui::DrawImGuiVector4Control(const Property& property, Vector4* value)
{
    ImGui::DragFloat4(property.name.c_str(), &value->x);
}

void ImGui::DrawImGuiFloatControl(const Property& property, float* value)
{
    ImGui::DragFloat(property.name.c_str(), value);
}

void ImGui::DrawImGuiIntControl(const Property& property, int* value)
{
    ImGui::DragInt(property.name.c_str(), value);
}
