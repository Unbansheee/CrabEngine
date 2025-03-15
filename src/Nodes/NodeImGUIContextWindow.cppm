module;

#pragma once

#include "ReflectionMacros.h"
#include "imgui/imgui.h"


export module node_imgui_context_window;
export import node_window;
import class_db;

export class NodeImGUIContextWindow : public NodeWindow
{
public:
    CRAB_CLASS(NodeImGUIContextWindow, NodeWindow)

    void EnterTree() override;
    void Update(float dt) override;
    void DrawGUI() override;

protected:
    ImGuiContext* imguiContext = nullptr;
};
