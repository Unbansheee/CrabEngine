//
// Created by Vinnie on 20/03/2025.
//
module;
#include "ReflectionMacros.h"

export module Engine.Node.CollisionObject3D;
import Engine.Node.Node3D;
import Engine.Physics.Body;
import Engine.Physics.Jolt;
import Engine.Node.CollisionShape3D;

export class NodeCollisionObject3D : public Node3D
{
  CRAB_ABSTRACT_CLASS(NodeCollisionObject3D, Node3D)

  void Init() override;
  void Ready() override;
  void Update(float dt) override;
  void EnterTree() override;
  void ExitTree() override;

  virtual void OnCollisionBegin(CollisionEvent event);

  std::vector<NodeCollisionShape3D*> GetCollisionShapes();
protected:
  virtual JPH::EMotionType GetMotionType() = 0;
  virtual JPH::ObjectLayer GetObjectLayer() = 0;


  void RebuildBody();
  void InvalidateBody();
  PhysicsBody body;

};





