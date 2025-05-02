module Engine.SceneTree;
import Engine.Application;
import Engine.Node;


SceneTree::~SceneTree() {
    Clear();
}

void SceneTree::Begin()
{
    physicsWorld = std::make_unique<PhysicsWorld>();
    physicsWorld->Init();
    if (root)
    {
        RegisterNode(root.get());
        ReadyNode(root.get());
    }
    physicsWorld->OptimizeBroadPhase();
    hasBegun = true;
}

void SceneTree::Update(float dt)
{
    auto& app = Application::Get();
    if (bUsePhysics) {
        physicsWorld->Update(dt, 1, app.GetPhysicsAllocator(), app.GetJobSystem());
    }

    if (root)
    {
        UpdateNode(root.get(), dt, true);
    }
}

void SceneTree::SetUsePhysics(bool usePhysics) {
    bUsePhysics = usePhysics;
}

void SceneTree::Clear() {
    if (root) root->ExitTree();
    root = nullptr;
    physicsWorld = nullptr;
}

Node* SceneTree::GetNodeByID(UID id)
{
    if (nodeMap.contains(id))
    {
        return nodeMap[id];
    }
    return nullptr;
}

void SceneTree::RegisterNode(Node* node)
{
    if (!nodeMap.contains(node->GetID()))
    {
        nodeMap[node->GetID()] = node;
        node->tree = this;
        node->isInTree = true;
        auto children = node->GetChildrenSafe();
        for (auto& child : children) {
            if (!child.IsValid()) continue;
            RegisterNode(child.Get());
        }

        if (node->scriptInstance) {
            node->scriptInstance->Call<void>(L"EnterTree");
        }
        else {
            node->EnterTree();
        }
    }
}

void SceneTree::UnregisterNode(Node* node)
{
    if (nodeMap.contains(node->GetID()))
    {
        nodeMap.erase(node->GetID());
        node->tree = nullptr;
        node->isInTree = false;

        auto children = node->GetChildrenSafe();
        for (auto& child : children) {
            UnregisterNode(child.Get());
        }

        if (node->scriptInstance) {
            node->scriptInstance->Call<void>(L"ExitTree");
        }
        else {
            node->ExitTree();
        }
    }
}

void SceneTree::UpdateNode(Node* n, float dt, bool recursive)
{
    if (n->scriptInstance) {
        n->scriptInstance->Call<void>(L"Update", dt);
    }
    else {
        n->Update(dt);
    }

    if (recursive)
    {
        for (auto& child : n->GetChildrenSafe()) {
            if (child) {
                UpdateNode(child.Get(), dt, recursive);
            }
        }
    }
}

void SceneTree::ReadyNode(Node* n)
{
    n->ForEachChild([this](Node* child)
    {
        ReadyNode(child);
    });

    if (!n->isReady)
    {
        if (n->scriptInstance) {
            n->scriptInstance->Call<void>(L"Ready");
        }
        else {
            n->Ready();
        }
        n->isReady = true;
    }
}


