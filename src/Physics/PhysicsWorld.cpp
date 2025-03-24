//
// Created by Vinnie on 20/03/2025.
//
module;
#include <cassert>

#include "Jolt/Jolt.h"
#include "Jolt/Core/IssueReporting.h"


module physics_world;
import Engine.Physics.Jolt;


PhysicsWorld::PhysicsWorld() {

}

void PhysicsWorld::Init() {
    const JPH::uint cMaxBodies = 65536;
    const JPH::uint cNumBodyMutexes = 0;
    const JPH::uint cMaxBodyPairs = 65536;
    const JPH::uint cMaxContactConstraints = 10240;

    physicsSystem.Init(cMaxBodies, cNumBodyMutexes, cMaxBodyPairs, cMaxContactConstraints, broadPhaseLayerInterface, object_vs_broadphase_layer_filter, object_vs_object_layer_filter);
    physicsSystem.SetBodyActivationListener(&bodyActivationListener);
    physicsSystem.SetContactListener(&contactListener);
    physicsSystem.SetGravity({0.f, 0.f, -9.81f});
}

void PhysicsWorld::OptimizeBroadPhase() {
    physicsSystem.OptimizeBroadPhase();
}

void PhysicsWorld::Update(float deltaTime, int collisionSteps, JPH::TempAllocator *temp_allocator,
                          JPH::JobSystem *job_system) {
    physicsSystem.Update(deltaTime, collisionSteps, temp_allocator, job_system);
}

PhysicsBody PhysicsWorld::AddBody(JPH::BodyCreationSettings &settings, JPH::EActivation activation) {
    JPH::BodyInterface &body_interface = physicsSystem.GetBodyInterface();
    JPH::Body* body = body_interface.CreateBody(settings);
    body_interface.AddBody(body->GetID(), activation);
    PhysicsBody b;
    b.body = body;
    b.bodyInterface = &body_interface;
    return b;
}

void PhysicsWorld::RemoveBody(PhysicsBody &body) {
    JPH::BodyInterface &body_interface = physicsSystem.GetBodyInterface();
    body_interface.RemoveBody(body.body->GetID());
}
