module node_camera_3d;
import node_window;
import glm;
//#include "glm/glm.hpp"


void NodeCamera3D::EnterTree()
{
    Node3D::EnterTree();
    if (auto w = GetAncestorOfType<NodeWindow>())
    {
        w->ActiveCamera = this;
    }
}

void NodeCamera3D::ExitTree()
{
    Node3D::ExitTree();
    if (auto w = GetAncestorOfType<NodeWindow>())
    {
        if (w->ActiveCamera == this)
        {
            w->ActiveCamera = nullptr;
        }
    }
}

Matrix4 NodeCamera3D::GetViewMatrix() const
{
    //return glm::lookAt(GetGlobalPosition(), GetGlobalPosition() + GetForwardVector(), {0, 0, 1});

    //glm::mat4 view = glm::translate(glm::mat4(1.0f), GetGlobalPosition() * -1.0f) * glm::inverse(glm::mat4_cast(GetGlobalOrientation()));
    return glm::inverse(GetTransform().GetWorldModelMatrix());
}
