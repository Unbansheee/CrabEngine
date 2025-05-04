//
// Created by Vinnie on 20/03/2025.
//
module Engine.Node.CollisionShape3D;
import Engine.Physics.JoltConversions;
import Engine.GFX.Renderer;
import Engine.Resource;
import Engine.Resource.Mesh;

void NodeBoxShape3D::Render(Renderer &renderer) {
    NodeCollisionShape3D::Render(renderer);

    if (renderer.bDebugDrawEnabled) {
        static std::shared_ptr<MeshResource> mesh = ResourceManager::Load<MeshResource>("/engine/Shapes/cube.obj");

        Transform scaled;
        scaled.Scale = Dimensions;

        scaled.ModelMatrix = GetTransform().GetWorldModelMatrix() * scaled.GetLocalModelMatrix();
        //renderer.DrawMesh(mesh, std::get<0>(GetDebugMaterial()), GetTransform().GetWorldModelMatrix(), this);
        renderer.DrawMesh(mesh, std::get<1>(GetDebugMaterial()), scaled.GetWorldModelMatrix(), this);
    }

}

JPH::ShapeRefC NodeBoxShape3D::GetShape() const {
    JPH::BoxShapeSettings boxSettings(glm_to_jolt(Dimensions));
    boxSettings.SetEmbedded();
    boxSettings.SetDensity(Density);
    auto s = boxSettings.Create();

    if (s.HasError()) {
        std::cout << s.GetError() << std::endl;
    }
    return s.Get();
}

JPH::ShapeRefC NodeCollisionShape3D::GetShapeTree() {
    JPH::StaticCompoundShapeSettings s;
    ForEachChildOfType<NodeCollisionShape3D>([&s](NodeCollisionShape3D* node) {
        s.AddShape(glm_to_jolt(node->GetPosition()), glm_to_jolt(node->GetOrientation()), node->GetShapeTree());
    });
    s.SetEmbedded();

    auto myshape = GetShape();
    JPH::ScaledShapeSettings scaledShapeSettings;
    auto sc = GetGlobalScale();
    scaledShapeSettings.mScale = glm_to_jolt(sc);
    scaledShapeSettings.mInnerShapePtr = myshape;
    scaledShapeSettings.SetEmbedded();
    auto scaledShape = scaledShapeSettings.Create();
    scaledShape.Get()->SetEmbedded();
    if (scaledShape.HasError()) {
        std::cout << scaledShape.GetError() << std::endl;
    }
    s.AddShape({}, {0, 0, 0, 1}, scaledShape.Get());

    auto shapeTree = s.Create();
    if (shapeTree.HasError()) {
        std::cout << shapeTree.GetError() << std::endl;
    }
    return shapeTree.Get();
}

void NodeSphereShape3D::Render(Renderer &renderer) {
    NodeCollisionShape3D::Render(renderer);

    if (renderer.bDebugDrawEnabled) {

        static std::shared_ptr<MeshResource> mesh = ResourceManager::Load<MeshResource>("/engine/Shapes/sphere.obj");

        Transform scaled;
        scaled.Scale = {Radius, Radius, Radius};
        scaled.ModelMatrix = GetTransform().GetWorldModelMatrix() * scaled.GetLocalModelMatrix();

        //renderer.DrawMesh(mesh, std::get<0>(GetDebugMaterial()), GetTransform().GetWorldModelMatrix(), this);
        renderer.DrawMesh(mesh, std::get<1>(GetDebugMaterial()), scaled.GetWorldModelMatrix(), this);

    }
}

JPH::ShapeRefC NodeSphereShape3D::GetShape() const {
    JPH::SphereShapeSettings settings(Radius);
    settings.SetEmbedded();
    settings.SetDensity(Density);
    auto s = settings.Create();
    if (s.HasError()) {
        std::cout << s.GetError() << std::endl;
    }
    return s.Get();
}

void NodeCapsuleShape3D::Render(Renderer &renderer) {
    NodeCollisionShape3D::Render(renderer);

    static std::shared_ptr<MeshResource> sphere = ResourceManager::Load<MeshResource>("/engine/Shapes/sphere.obj");
    static std::shared_ptr<MeshResource> cylinder = ResourceManager::Load<MeshResource>("/engine/Shapes/cylinder.obj");

    if (renderer.bDebugDrawEnabled) {
        float r2 = Radius;
        Transform up;
        up.Position = {0, HalfHeight, 0};
        up.Scale = {r2, r2, r2};

        Transform down;
        down.Position = {0, -HalfHeight, 0};
        down.Scale = {r2, r2, r2};


        Transform cyl;
        cyl.Scale = {r2, r2, HalfHeight};
        cyl.Orientation = glm::angleAxis(glm::radians(90.f), Vector3{1.f, 0.f, 0.f});

        up.ModelMatrix = GetTransform().GetWorldModelMatrix() * up.GetLocalModelMatrix();
        down.ModelMatrix = GetTransform().GetWorldModelMatrix() * down.GetLocalModelMatrix();
        cyl.ModelMatrix = GetTransform().GetWorldModelMatrix() * cyl.GetLocalModelMatrix();

        auto mat = GetDebugMaterial();

        //renderer.DrawMesh(sphere, std::get<0>(mat), up.GetWorldModelMatrix(), this);
        renderer.DrawMesh(sphere, std::get<1>(mat), up.GetWorldModelMatrix(), this);

        //renderer.DrawMesh(sphere, std::get<0>(mat), down.GetWorldModelMatrix(), this);
        renderer.DrawMesh(sphere, std::get<1>(mat), down.GetWorldModelMatrix(), this);

        //renderer.DrawMesh(cylinder, std::get<0>(mat), cyl.GetWorldModelMatrix(), this);
        renderer.DrawMesh(cylinder, std::get<1>(mat), cyl.GetWorldModelMatrix(), this);
    }
}

JPH::ShapeRefC NodeCapsuleShape3D::GetShape() const {
    JPH::CapsuleShapeSettings settings(HalfHeight, Radius);
    settings.SetEmbedded();
    settings.SetDensity(Density);
    auto s = settings.Create();
    if (s.HasError()) {
        std::cout << s.GetError() << std::endl;
    }
    return s.Get();
}
