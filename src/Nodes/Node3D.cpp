//#include "Node3D.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/quaternion.hpp>

//#include "Core/ClassDB.cppm"

module Engine.Node.Node3D;
import Engine.Reflection.ClassDB;
import Engine.Transform;
import Engine.Types;


void Node3D::UpdateTransform()
{
    if (auto parent = GetParent())
    {
        transform.ModelMatrix = parent->GetTransform().GetWorldModelMatrix() * transform.GetLocalModelMatrix();
    }
    else
    {
        transform.ModelMatrix = transform.GetLocalModelMatrix();
    }

    Node::UpdateTransform();
}

void Node3D::SetPosition(const Vector3& Pos)
{
    transform.Position = Pos;
    UpdateTransform();
}

void Node3D::SetScale(const Vector3& Scale)
{
    transform.Scale = Scale;
    UpdateTransform();
}

void Node3D::SetOrientation(const Quat& Orientation)
{
    transform.Orientation = normalize(Orientation);
    UpdateTransform();
}


Vector3 Node3D::GetPosition() const
{
    return transform.Position;
} 

Vector3 Node3D::GetScale() const
{
    return transform.Scale;
}

Quat Node3D::GetOrientation() const
{
    return transform.Orientation;
}

Vector3 Node3D::GetGlobalPosition() const
{
    if (GetParent())
    {
        return GetParentTransform().LocalToWorldPosition(GetPosition());
    }
    else
    {
        return GetPosition();
    }
}

Vector3 Node3D::GetGlobalScale() const
{
    if (GetParent())
    {
        return GetParentTransform().LocalToWorldScale(GetScale());
    }
    else
    {
        return GetScale();
    }
}

Quat Node3D::GetGlobalOrientation() const
{
    if (GetParent())
    {
        return GetParentTransform().LocalToWorldRotation(GetOrientation());
    }
    else
    {
        return GetOrientation();
    }
}

void Node3D::SetGlobalPosition(const Vector3& NewPosition)
{
    if (GetParent())
    {
        transform.Position = GetParentTransform().WorldToLocalPosition(NewPosition);
    }
    else
        transform.Position = NewPosition;

    UpdateTransform();

}

void Node3D::SetGlobalScale(const Vector3& NewScale)
{
    if (GetParent())
    {
        transform.Scale = GetParentTransform().WorldToLocalScale(NewScale);
    }
    else
        transform.Scale = NewScale;

    UpdateTransform();

}

void Node3D::SetGlobalOrientation(const Quat& NewOrientation)
{
    if (GetParent())
    {
        transform.Orientation = GetParentTransform().WorldToLocalRotation(NewOrientation);
    }
    else
        transform.Orientation = NewOrientation;

    UpdateTransform();
}

Vector3 Node3D::GetForwardVector() const
{
    return transform.LocalToWorldDirection({ 1, 0, 0 });
}

Vector3 Node3D::GetUpVector() const
{
    return transform.LocalToWorldDirection({ 0, 0, 1 });
}

Vector3 Node3D::GetRightVector() const
{
    return transform.LocalToWorldDirection({ 0, 1, 0 });
}

Transform Node3D::GetTransform() const
{
    return transform;
}