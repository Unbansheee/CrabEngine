//
// Created by Vinnie on 20/03/2025.
//

export module Engine.Physics.Listeners;
import Engine.Physics.Jolt;
import std;
import Engine.Physics.Body;
import Engine.Types;
import Engine.Physics.JoltConversions;


// An example contact listener
export class CrabContactListener : public JPH::ContactListener
{
public:
    // See: ContactListener
    virtual JPH::ValidateResult	OnContactValidate(const JPH::Body &inBody1, const JPH::Body &inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult &inCollisionResult) override
    {
        //std::cout << "Contact validate callback" << std::endl;

        // Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
        return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
    }

    virtual void OnContactAdded(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
    {
        std::cout << "A contact was added" << std::endl;
        CollisionObjectUserData* data1 = reinterpret_cast<CollisionObjectUserData*>(inBody1.GetUserData());
        CollisionObjectUserData* data2 = reinterpret_cast<CollisionObjectUserData*>(inBody2.GetUserData());

        CollisionEvent e1;
        e1.OtherBody = data2->Body;
        auto hitloc1 = inManifold.GetWorldSpaceContactPointOn1(0);
        e1.HitLocation = jolt_to_glm(hitloc1);
        auto norm = inManifold.mWorldSpaceNormal;
        e1.HitNormal = jolt_to_glm(norm);

        CollisionEvent e2;
        e2.OtherBody = data1->Body;
        auto hitloc2 = inManifold.GetWorldSpaceContactPointOn2(0);
        e2.HitLocation = jolt_to_glm(hitloc2);
        e2.HitNormal = e1.HitNormal;

        data1->Body->OnCollisionBegin(e1);
        data2->Body->OnCollisionBegin(e2);
    }

    virtual void OnContactPersisted(const JPH::Body &inBody1, const JPH::Body &inBody2, const JPH::ContactManifold &inManifold, JPH::ContactSettings &ioSettings) override
    {
        //std::cout << "A contact was persisted" << std::endl;
    }

    virtual void OnContactRemoved(const JPH::SubShapeIDPair &inSubShapePair) override
    {
        std::cout << "A contact was removed" << std::endl;
    }
};


// An example activation listener
export class CrabBodyActivationListener : public JPH::BodyActivationListener
{
public:
    virtual void OnBodyActivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override
    {
        std::cout << "A body got activated" << std::endl;
    }

    virtual void OnBodyDeactivated(const JPH::BodyID &inBodyID, JPH::uint64 inBodyUserData) override
    {
        std::cout << "A body went to sleep" << std::endl;
    }
};