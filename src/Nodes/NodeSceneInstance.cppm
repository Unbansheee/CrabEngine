//
// Created by Vinnie on 28/04/2025.
//


module;
#include "ReflectionMacros.h"

export module Engine.Node.NodeSceneInstance;
import Engine.Node;
import Engine.Node.Node3D;
import Engine.Resource.Scene;
import Engine.Object.Ref;

class NodeSceneInstance : public Node3D {
    CRAB_CLASS(NodeSceneInstance, Node3D)
    CLASS_FLAG(EditorVisible)
    BEGIN_PROPERTIES
        ADD_PROPERTY("Scene", Scene)
    END_PROPERTIES

    std::shared_ptr<SceneResource> Scene;
    ObjectRef<Node> ChildScene;
public:
    virtual void Ready() override;
    void EnterTree() override;
    void ExitTree() override;
    void OnPropertySet(Property &prop) override;
};

void NodeSceneInstance::Ready() {
    Node3D::Ready();
}

void NodeSceneInstance::EnterTree() {
    Node3D::EnterTree();

    if (GetTree()->IsInEditor()) {
        if (Scene) {
            ChildScene = AddChild(Scene->Instantiate());
            ChildScene->AddFlag(ObjectFlags::Transient);
            ChildScene->AddFlag(ObjectFlags::HiddenFromTree);
        }
    }
    else {
        if (Scene) {
            auto child = GetParent()->AddChild(Scene->Instantiate());
            if (auto node3d = Object::Cast<Node3D>(child)) {
                node3d->SetGlobalPosition(GetGlobalPosition());
                node3d->SetGlobalOrientation(GetGlobalOrientation());
                node3d->SetGlobalScale(GetGlobalScale());
            };
        }

        RemoveFromParent();
    }
}

void NodeSceneInstance::ExitTree() {
    Node3D::ExitTree();

    if (ChildScene.IsValid()) {
        ChildScene->RemoveFromParent();
    }

}

void NodeSceneInstance::OnPropertySet(Property &prop) {
    Node3D::OnPropertySet(prop);

    if (GET_PROPERTY_NAME(Scene) == prop.name) {
        if (IsInTree() && GetTree()->IsInEditor()) {
            if (ChildScene.IsValid()) {
                ChildScene->RemoveFromParent();
            }
        }
    }

    if (IsInTree() && GetTree()->IsInEditor()) {
        if (Scene) {
            ChildScene = AddChild(Scene->Instantiate());
            ChildScene->AddFlag(ObjectFlags::Transient);
            ChildScene->AddFlag(ObjectFlags::HiddenFromTree);
        }
    }
}
