module;

#pragma once

export module scene_tree;
import uid;
import <memory>;
import <unordered_map>;

export class Node;

export class SceneTree
{
    friend class Node;
public:
    void Begin();
    void Update(float dt);

    template<typename T>
    T* SetRoot(std::unique_ptr<T> root)
    {
        this->root = std::move(root);
        if (hasBegun)
            RegisterNode(this->root.get());

        return static_cast<T*>(this->root.get());
    }

    template<typename T>
    T* GetRoot() const {return root.get();}

    Node* GetNodeByID(UID id);
    
private:
    void RegisterNode(Node* node);
    void UnregisterNode(Node* node);
    
    std::unique_ptr<Node> root;
    std::unordered_map<UID, Node*> nodeMap;
    bool hasBegun = false;

private:
    void UpdateNode(Node* n, float dt, bool recursive = true);
    void ReadyNode(Node* n);
};



