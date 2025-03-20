//#include "Node3D.h"
//#include <glm/glm.hpp>
//#include <glm/gtc/quaternion.hpp>

//#include "Core/ClassDB.cppm"

module node_3d;
import class_db;
import transform;
import crab_types;


void Node3D::UpdateTransform()
{
    if (Parent)
    {
        transform.ModelMatrix = Parent->GetTransform().GetWorldModelMatrix() * transform.GetLocalModelMatrix();
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
    if (Parent)
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
    if (Parent)
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
    if (Parent)
    {
        // TODO: test this
        return GetParentTransform().LocalToWorldRotation(GetOrientation());
    }
    else
    {
        return GetOrientation();
    }
}

void Node3D::SetGlobalPosition(const Vector3& NewPosition)
{
    if (Parent)
    {
        transform.Position = GetParentTransform().WorldToLocalPosition(NewPosition);
    }
    else
        transform.Position = NewPosition;

    UpdateTransform();

}

void Node3D::SetGlobalScale(const Vector3& NewScale)
{
    if (Parent)
    {
        transform.Scale = GetParentTransform().WorldToLocalScale(NewScale);
    }
    else
        transform.Scale = NewScale;

    UpdateTransform();

}

void Node3D::SetGlobalOrientation(const Quat& NewOrientation)
{
    if (Parent)
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