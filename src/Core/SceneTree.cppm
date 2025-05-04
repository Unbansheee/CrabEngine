module;

#pragma once

export module Engine.SceneTree;
import Engine.UID;
import physics_world;
import <memory>;
import <unordered_map>;

// Holds a root node and is responsible for the traversal and ticking of the tree
// If this is destroyed, any nodes inside will also be destroyed
// Contains the Physics world
export class SceneTree
{
    friend class Node;
    friend class NodeEditorSceneRoot;
public:
    // Start the tree/scene
    void Begin();
    // Tick all nodes recursively
    void Update(float dt);
    // Set whether the physics scene should tick or not
    void SetUsePhysics(bool usePhysics);
    // Kill physics world and root node
    void Clear();
    ~SceneTree();
    PhysicsWorld& GetPhysicsWorld() {return *physicsWorld.get();}

    // Replace the current root node with a new root. Any children of the old root will be eliminated
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

    // Get the current Root node
    template<typename T = Node>
    T* GetRoot() const {return root.get();}

    //Set the root to a new root node, but return the old one.
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

    // Is the tree in the editor? This does not mean it is the scene tree of the editor, but
    // That it is instantiated in the editor and not running
    // TODO: maybe rename to IsRunning and invert?
    bool IsInEditor() const { return bIsInEditor; }
private:
    void RegisterNode(Node* node); // Add node to nodemap and run any relevant EnterTree/Ready functions
    void UnregisterNode(Node* node); // Remove node from nodemap and run any relevant ExitTree functions
    std::unique_ptr<Node> root; // Rootnode
    std::unordered_map<UID, Node*> nodeMap; // UID : Node lookup
    bool hasBegun = false; // Has Ready been called
    bool bUsePhysics = true; // Should the physics world tick
    bool bIsInEditor = false;
    std::unique_ptr<PhysicsWorld> physicsWorld; // Jolt physics world
private:
    void UpdateNode(Node* n, float dt, bool recursive = true);
    void ReadyNode(Node* n);
};




