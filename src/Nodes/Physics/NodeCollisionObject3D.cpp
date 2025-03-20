//
// Created by Vinnie on 20/03/2025.
//
module node_collision_object_3d;
import jolt;
import node_collision_shape_3d;
import physics.layers;
import jolt_conversions;


void NodeCollisionObject3D::Init() {
    Node3D::Init();
}

void NodeCollisionObject3D::Ready() {
    Node3D::Ready();

    RebuildBody();
}

void NodeCollisionObject3D::ExitTree() {
    Node3D::ExitTree();
    InvalidateBody();
}

void NodeCollisionObject3D::EnterTree() {
    Node3D::EnterTree();
    if (isReady) {
        RebuildBody();
    }
}

void NodeCollisionObject3D::Update(float dt) {
    Node3D::Update(dt);
    if (body.IsValid() && body.IsActive()) {
        SetGlobalPosition(body.GetPosition());
        SetGlobalOrientation(body.GetOrientation());
    }
}

std::vector<NodeCollisionShape3D*> NodeCollisionObject3D::GetCollisionShapes() {
    std::vector<NodeCollisionShape3D*> shapes;
    ForEachChildOfType<NodeCollisionShape3D>([&shapes](NodeCollisionShape3D* node) {
        shapes.push_back(node);
    });

    return shapes;
}

void NodeCollisionObject3D::RebuildBody() {
    JPH::BodyCreationSettings settings;

    InvalidateBody();
    auto shapes = GetCollisionShapes();
    if (shapes.empty()) return;
    JPH::StaticCompoundShapeSettings s;
    s.SetEmbedded();
    for (auto shape : shapes)
    {
        s.AddShape(glm_to_jolt(shape->GetPosition()), glm_to_jolt(shape->GetOrientation()), shape->GetShapeTree());
    };
    auto shape = s.Create();
    settings.SetShape(shape.Get());

    CollisionObjectUserData* data = new CollisionObjectUserData();
    data->Body = &body;
    settings.mUserData = (JPH::uint64)data;

    auto pos = GetGlobalPosition();
    auto orientation = GetGlobalOrientation();
    settings.mPosition = glm_to_jolt(pos);
    settings.mRotation = glm_to_jolt(orientation);
    settings.mMotionType = GetMotionType();
    settings.mObjectLayer = GetObjectLayer();
    settings.mAllowSleeping = false;

    body = GetTree()->GetPhysicsWorld().AddBody(settings, JPH::EActivation::Activate);
    body.OnCollisionBegin.connect(this, &NodeCollisionObject3D::OnCollisionBegin);
}

void NodeCollisionObject3D::InvalidateBody() {
    if (body.IsValid()) {
        body.OnCollisionBegin.clear();
    }

    body.Invalidate();
}

void NodeCollisionObject3D::OnCollisionBegin(CollisionEvent event) {
    std::cout << "Node received collision event!\n";
}
