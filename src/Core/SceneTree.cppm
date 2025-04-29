module;

#pragma once

export module Engine.SceneTree;
import Engine.UID;
import physics_world;
import <memory>;
import <unordered_map>;


export class SceneTree
{
    friend class Node;
    friend class NodeEditorSceneRoot;
public:
    void Begin();
    void Update(float dt);
    void SetUsePhysics(bool usePhysics);
    void Clear();
    ~SceneTree();
    PhysicsWorld& GetPhysicsWorld() {return *physicsWorld.get();}

    template<typename T>
    T* SetRoot(std::unique_ptr<T> newRoot)
    {
        if (this->root) {
            UnregisterNode(this->root.get());
        }
        this->root = std::move(newRoot);
        RegisterNode(this->root.get());

        return static_cast<T*>(this->root.get());
    }

    template<typename T = Node>
    T* GetRoot() const {return root.get();}

    template<typename T = Node>
    std::unique_ptr<T> SwapRoot(std::unique_ptr<T> new_root) {
        if (this->root) {
            UnregisterNode(this->root.get());
        }
        auto r = std::move(root);
        SetRoot(std::move(new_root));
        return r;
    }

    Node* GetNodeByID(UID id);
    bool IsInEditor() const { return bIsInEditor; }
private:
    void RegisterNode(Node* node);
    void UnregisterNode(Node* node);
    void CurrentRootExitingTree();
    std::unique_ptr<Node> root;
    std::unordered_map<UID, Node*> nodeMap;
    bool hasBegun = false;
    bool bUsePhysics = true;
    bool bIsInEditor = false;
    std::unique_ptr<PhysicsWorld> physicsWorld;
private:
    void UpdateNode(Node* n, float dt, bool recursive = true);
    void ReadyNode(Node* n);
};




