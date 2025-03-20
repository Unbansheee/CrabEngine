//
// Created by Vinnie on 20/03/2025.
//
module;
#include "Jolt/Jolt.h"
#include "Jolt/Core/IssueReporting.h"
#include "Jolt/Core/Memory.h"
#include "Jolt/RegisterTypes.h"
#include "Jolt/Core/TempAllocator.h"
#include "Jolt/Core/JobSystem.h"
#include "Jolt/Core/JobSystemSingleThreaded.h"
#include "Jolt/Core/JobSystemThreadPool.h"
#include "Jolt/Core/TickCounter.h"
#include "Jolt/Physics/PhysicsSettings.h"
#include "Jolt/Physics/PhysicsSystem.h"
#include "Jolt/Physics/Body/BodyActivationListener.h"
#include "Jolt/Physics/Collision/ContactListener.h"
#include "Jolt/Physics/Collision/Shape/BoxShape.h"
#include "Jolt/Physics/Body/BodyCreationSettings.h"
#include "Jolt/Physics/Collision/Shape/CompoundShape.h"
#include "Jolt/Physics/Collision/Shape/SphereShape.h"
#include "Jolt/Physics/Collision/Shape/Shape.h"
#include "Jolt/Core/Factory.h"
#include "Jolt/Core/HashCombine.h"
#include "Jolt/Physics/Collision/Shape/MutableCompoundShape.h"
#include "Jolt/Physics/Collision/Shape/ScaledShape.h"
#include "Jolt/Physics/Collision/Shape/StaticCompoundShape.h"


export module jolt;
export import "Jolt/Jolt.h";

export namespace JPH {
    using JPH::Factory;
    using JPH::TempAllocator;
    using JPH::TempAllocatorImpl;
    using JPH::JobSystem;
    using JPH::JobSystemThreadPool;
    using JPH::JobSystemSingleThreaded;
    using JPH::JobSystemWithBarrier;
    using JPH::PhysicsSystem;
    using JPH::BodyInterface;
    using JPH::BodyActivationListener;
    using JPH::BodyID;
    using JPH::ContactListener;
    using JPH::ValidateResult;
    using JPH::Body;
    using JPH::CollideShapeResult;
    using JPH::ContactSettings;
    using JPH::ContactManifold;
    using JPH::SubShapeID;
    using JPH::SubShapeIDPair;
    using JPH::ObjectVsBroadPhaseLayerFilter;
    using JPH::ObjectLayer;
    using JPH::BroadPhaseLayer;
    using JPH::ObjectLayerPairFilter;
    using JPH::BroadPhaseLayerInterface;
    using JPH::Quat;
    using JPH::BodyCreationSettings;
    using JPH::RVec3;
    using JPH::Vec3;
    using JPH::BoxShape;
    using JPH::BoxShapeSettings;
    using JPH::Shape;
    using JPH::ShapeSettings;
    using JPH::ShapeRefC;
    using JPH::EMotionType;
    using JPH::EActivation;
    using JPH::SphereShape;
    using JPH::SphereShapeSettings;
    using JPH::RVec3Arg;
    using JPH::uint;
    using JPH::uint8;
    using JPH::uint16;
    using JPH::uint32;
    using JPH::uint64;
    using JPH::cMaxPhysicsJobs;
    using JPH::cMaxPhysicsBarriers;
    using JPH::CompoundShape;
    using JPH::CompoundShapeSettings;
    using JPH::Ref;
    using JPH::MutableCompoundShape;
    using JPH::MutableCompoundShapeSettings;
    using JPH::StaticCompoundShape;
    using JPH::StaticCompoundShapeSettings;
    using JPH::ScaledShapeSettings;
    using JPH::ScaledShape;

    void RegisterJoltTypes() {
        JPH::RegisterTypes();
    };
    void UnregisterJoltTypes() {
        JPH::UnregisterTypes();
    };
}