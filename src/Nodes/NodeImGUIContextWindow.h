#pragma once
#include "NodeWindow.h"

class NodeImGUIContextWindow : public NodeWindow
{
public:
    NodeImGUIContextWindow() = default;

    NodeImGUIContextWindow(const std::string& name)
        : NodeWindow(name)
    {
    }

    void Begin() override;
    void Update(float dt) override;
    void DrawGUI() override;

protected:
    ImGuiContext* imguiContext = nullptr;
};
