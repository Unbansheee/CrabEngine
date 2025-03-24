//
// Created by Vinnie on 20/03/2025.
//

export module Engine.Physics.Body;
import Engine.Physics.Jolt;
import Engine.Types;
import uid;
import rocket;
import std;
import Engine.Physics.JoltConversions;

export class PhysicsBody;

export struct CollisionEvent {
    PhysicsBody* OtherBody;
    Vector3 HitNormal;
    Vector3 HitLocation;
};

export struct CollisionObjectUserData {
    PhysicsBody* Body;
};

class PhysicsBody
{
    friend class PhysicsWorld;
public:
    void SetLinearVelocity(const Vector3& velocity);
    void SetPosition(const Vector3& pos, JPH::EActivation mode = JPH::EActivation::DontActivate);
    Vector3 GetLinearVelocity() const;
    Vector3 GetPosition() const;
    Quat GetOrientation() const;

    void SetRestitution(float restitution);
    float GetRestitution() const;

    void SetFriction(float friction);
    float GetFriction() const;

    bool IsValid() const { return body && bodyInterface; };
    void Invalidate();
    bool IsActive() const;
    bool IsAdded() const;

    void Activate();
    void Deactivate();

    rocket::signal<void(CollisionEvent)> OnCollisionBegin;

private:
    JPH::BodyInterface* bodyInterface;
    JPH::Body* body;
};

void PhysicsBody::SetLinearVelocity(const Vector3& velocity) {
    bodyInterface->SetLinearVelocity(body->GetID(), glm_to_jolt(velocity));
}

void PhysicsBody::SetPosition(const Vector3 &pos, JPH::EActivation mode) {
    bodyInterface->SetPosition(body->GetID(), glm_to_jolt(pos), mode);
}

Vector3 PhysicsBody::GetLinearVelocity() const {
    auto vel = bodyInterface->GetLinearVelocity(body->GetID());
    return jolt_to_glm(vel);
}

Vector3 PhysicsBody::GetPosition() const {
    auto vel = bodyInterface->GetPosition(body->GetID());
    return jolt_to_glm(vel);
}

Quat PhysicsBody::GetOrientation() const {
    auto q = bodyInterface->GetRotation(body->GetID());
    return Quat{q.GetW(), q.GetX(), q.GetY(), q.GetZ()};
}

void PhysicsBody::SetRestitution(float restitution) {
    bodyInterface->SetRestitution(body->GetID(), restitution);
}

float PhysicsBody::GetRestitution() const {
    return bodyInterface->GetRestitution(body->GetID());
}

void PhysicsBody::SetFriction(float friction) {
    bodyInterface->SetFriction(body->GetID(), friction);
}

float PhysicsBody::GetFriction() const {
    return bodyInterface->GetFriction(body->GetID());
}

void PhysicsBody::Invalidate() {
    if (bodyInterface && body) {
        std::cout << "Invalidated body" << std::endl;
        bodyInterface->RemoveBody(body->GetID());
        bodyInterface->DestroyBody(body->GetID());
    }
    bodyInterface = nullptr;
    body = nullptr;
}

bool PhysicsBody::IsActive() const {
    return bodyInterface->IsActive(body->GetID());
}

bool PhysicsBody::IsAdded() const {
    return bodyInterface->IsAdded(body->GetID());
}

void PhysicsBody::Activate() {
    bodyInterface->ActivateBody(body->GetID());
}

void PhysicsBody::Deactivate() {
    bodyInterface->DeactivateBody(body->GetID());
}




