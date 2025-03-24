module;

#pragma once

#include "ReflectionMacros.h"
#include "imgui.h"


export module Engine.Node.ImGuiContextWindow;
export import Engine.Node.Window;
import Engine.Reflection.ClassDB;


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
