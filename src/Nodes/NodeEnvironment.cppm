//
// Created by Vinnie on 28/04/2025.
//


module;
#include "ReflectionMacros.h"

export module Engine.Node.NodeEnvironment;
import Engine.Node;
import Engine.Types;
import Engine.GFX.Renderer;

export class NodeEnvironment : public Node {
    CRAB_CLASS(NodeEnvironment, Node)
    CLASS_FLAG(EditorVisible)
    BEGIN_PROPERTIES
        ADD_PROPERTY_FLAGS("BackgroundColour", BackgroundColour, PropertyFlags::ColorHint)
    END_PROPERTIES

public:
    void Render(Renderer &renderer) override;

    Vector4 BackgroundColour = {0, 0, 0, 1};
};

void NodeEnvironment::Render(Renderer &renderer) {
    Node::Render(renderer);
    renderer.SetClearColour(BackgroundColour);
}
