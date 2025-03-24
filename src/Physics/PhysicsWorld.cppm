//
// Created by Vinnie on 20/03/2025.
//

export module physics_world;
import Engine.Physics.Jolt;
import Engine.Physics.Layers;
import Engine.Physics.Listeners;
import Engine.Physics.Body;

export class PhysicsWorld
{
public:
    PhysicsWorld();
    void Init();
    void OptimizeBroadPhase();
    void Update(float deltaTime, int collisionSteps, JPH::TempAllocator* temp_allocator, JPH::JobSystem* job_system);

    PhysicsBody AddBody(JPH::BodyCreationSettings& settings, JPH::EActivation activation);
    void RemoveBody(PhysicsBody& body);
protected:
    JPH::PhysicsSystem physicsSystem;
    BPLayerInterfaceImpl broadPhaseLayerInterface;
    ObjectVsBroadPhaseLayerFilterImpl object_vs_broadphase_layer_filter;
    ObjectLayerPairFilterImpl object_vs_object_layer_filter;
    CrabBodyActivationListener bodyActivationListener;
    CrabContactListener contactListener;
};


