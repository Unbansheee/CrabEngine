#include "SceneTree.h"
#include "Node.h"

void SceneTree::Begin()
{
    RegisterNode(root.get());
    ReadyNode(root.get());

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
    if (!nodeMap.contains(node->id))
    {
        nodeMap[node->id] = node;
        node->tree = this;
        for (auto child : node->GetChildren<Node>())
        {
            RegisterNode(child);
        }
        
        node->EnterTree();
    }
}

void SceneTree::UnregisterNode(Node* node)
{
    if (nodeMap.contains(node->id))
    {
        nodeMap.erase(node->id);
        node->tree = nullptr;
        for (auto child : node->GetChildren<Node>())
        {
            UnregisterNode(child);
        }
        node->ExitTree();
    }
}

void SceneTree::UpdateNode(Node* n, float dt, bool recursive)
{
    n->Update(dt);
    
    if (recursive)
    {
        for (auto child : n->GetChildren())
        {
            UpdateNode(child, dt, recursive);
        }
    }
}

void SceneTree::ReadyNode(Node* n)
{
    for (auto child : n->GetChildren())
    {
        ReadyNode(child);
    }

    if (!n->isReady)
    {
        n->Ready();
        n->isReady = true;
    }
}


