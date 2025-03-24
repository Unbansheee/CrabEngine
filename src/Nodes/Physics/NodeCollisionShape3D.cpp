//
// Created by Vinnie on 20/03/2025.
//
module Engine.Node.CollisionShape3D;
import Engine.Physics.JoltConversions;


JPH::ShapeRefC NodeBoxShape3D::GetShape() const {
    JPH::BoxShapeSettings boxSettings(glm_to_jolt(Dimensions));
    boxSettings.SetEmbedded();
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

    auto tree = s.Create();
    if (tree.HasError()) {
        std::cout << tree.GetError() << std::endl;
    }
    return tree.Get();
}

JPH::ShapeRefC NodeSphereShape3D::GetShape() const {
    JPH::SphereShapeSettings settings(Radius);
    settings.SetEmbedded();
    auto s = settings.Create();
    if (s.HasError()) {
        std::cout << s.GetError() << std::endl;
    }
    return s.Get();
}
