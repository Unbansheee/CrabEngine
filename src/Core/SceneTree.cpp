module Engine.SceneTree;
import Engine.Node;
import Engine.Application;


SceneTree::~SceneTree() {
    root = nullptr;
    physicsWorld = nullptr;
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
        node->ForEachChild([this](Node* child)
        {
            RegisterNode(child);
        });

        node->EnterTree();
    }
}

void SceneTree::UnregisterNode(Node* node)
{
    if (nodeMap.contains(node->GetID()))
    {
        nodeMap.erase(node->GetID());
        node->tree = nullptr;
        node->isInTree = false;
        node->ForEachChild([this](Node* child)
        {
            UnregisterNode(child);
        });

        node->ExitTree();
    }
}

void SceneTree::UpdateNode(Node* n, float dt, bool recursive)
{
    n->Update(dt);
    
    if (recursive)
    {
        n->ForEachChild([this, dt, recursive](Node* child)
        {
            UpdateNode(child, dt, recursive);
        });
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
        n->Ready();
        n->isReady = true;
    }
}


