#pragma once
#include "NodeWindow.h"
#include "Core/ClassDB.h"


struct ImGuiContext;

class NodeImGUIContextWindow : public NodeWindow
{
public:
    CRAB_CLASS(NodeImGUIContextWindow, NodeWindow)

    void EnterTree() override;
    void Update(float dt) override;
    void DrawGUI() override;

protected:
    ImGuiContext* imguiContext = nullptr;
};
