#pragma once
#include "Node.h"
#include "Transform.h"

// 3D version of a Node
// Contains a Transform
// Can be translated, rotated, scaled

class Node3D : public Node
{
public:
    virtual void SetPosition(const Vector3& Pos);
    virtual void SetScale(const Vector3& Scale);
    virtual void SetOrientation(const Quat& Orientation);

    Vector3 GetPosition() const;
    Vector3 GetScale() const;
    Quat GetOrientation() const;

    Vector3 GetGlobalPosition() const;
    Vector3 GetGlobalScale() const;
    Quat GetGlobalOrientation() const;

    void SetGlobalPosition(const Vector3& NewPosition);
    void SetGlobalScale(const Vector3& NewScale);
    void SetGlobalOrientation(const Quat& NewOrientation);

    Vector3 GetForwardVector() const;
    Vector3 GetUpVector() const;
    Vector3 GetRightVector() const;

    Transform GetTransform() const override;
    void UpdateTransform() override;
protected:

    Transform transform;

private:

};