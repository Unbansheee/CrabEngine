#pragma once
#include "NodeWindow.h"
#include "Core/ClassDB.h"


class NodeImGUIContextWindow : public NodeWindow
{
public:
    void EnterTree() override;
    void Update(float dt) override;
    void DrawGUI() override;

protected:
    ImGuiContext* imguiContext = nullptr;
};
