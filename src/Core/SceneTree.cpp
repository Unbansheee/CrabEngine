#include "SceneTree.h"
#include "Node.h"

void SceneTree::Begin()
{
    if (root)
    {
        RegisterNode(root.get());
        ReadyNode(root.get());
    }
    
    hasBegun = true;
}

void SceneTree::Update(float dt)
{
    if (root)
    {
        UpdateNode(root.get(), dt, true);
    }
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


