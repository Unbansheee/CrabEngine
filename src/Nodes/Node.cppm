//
// Created by Vinnie on 22/02/2025.
module;
#pragma once

#include "ReflectionMacros.h"

export module Engine.Node;
export import Engine.Input;
import Engine.Object;
import Engine.Transform;
import Engine.SceneTree;
import Engine.Object.Ref;
import std;


export class RenderVisitor;
export class Renderer;

export class Node : public Object {
	friend class Renderer;

public:
	CRAB_CLASS(Node, Object)
	CLASS_FLAG(EditorVisible)
	BEGIN_PROPERTIES
		ADD_PROPERTY("Name", Name)
		ADD_PROPERTY("Hidden", isHidden)
	END_PROPERTIES

	BIND_METHOD(void, Ready)
	BIND_METHOD(void, EnterTree)
	BIND_METHOD(void, ExitTree)
	BIND_METHOD_PARAMS(void, Update, float dt, (dt))
	BIND_METHOD_PARAMS(void, SetName, char* name, (name))
	BIND_METHOD_PARAMS(InputResult, HandleInput, InputEventInterop event, (event))

	static void NativeGetName(ThisClass* ctx, wchar_t* outString);
	BIND_STATIC_METHOD(const wchar_t*, NativeGetName);

	virtual ~Node() override;
protected:
	friend class Application;
	friend class NodeWindow;
	friend class NodeImGUIContextWindow;
	friend class SceneTree;
	friend class Object;

	// Runs every frame
	virtual void Update(float dt) {	}

	// Draw ImGui content if it is a child of a NodeImGuiContextWindow
	virtual void DrawGUI() {};

	// Called when the node enters the SceneTree
	virtual void EnterTree() {};

	// Called when the node exits the SceneTree
	virtual void ExitTree() {};

	// Called when all of the child nodes are ready
	virtual void Ready() {};

	// Called upon initialization
	virtual void Init() {};

	// Called when traversed by the scene renderer. Allows the pushing of draw commands
	virtual void Render(Renderer& renderer);

	// Respond to input event. If Handled is returned, the input event will stop traversing the tree and will be counted as consumed
	virtual InputResult HandleInput(const InputEvent& event) { return InputResult::Ignored; }

public:
	// Initialize a raw node pointer as a unique_ptr. You generally should not do this, but it is needed in some niche scenarios
	template<typename T = Node>
	static std::unique_ptr<T> InitializeNode(T* raw, const std::string& name);

	// Instantiate a new node of type T
	template<typename T = Node>
	static std::unique_ptr<T> NewNode(const std::string& name = "Node");

	// Instantiate a new node using a reflected ClassType
	static std::unique_ptr<Node> NewNode(const ClassType& classType, const std::string& name = "Node")
	{
		Node* node = static_cast<Node*>(classType.Initializer());
		return InitializeNode(node, name);
	};

	// Duplicate this node and all of its children.
	// Duplication will copy over any reflected properties not marked as Transient
	std::unique_ptr<Node> Duplicate();

	// TODO: Remove Transform functions from Node and keep it only on Node3D
	// Returns the Transform data of this Node
	virtual Transform GetTransform() const;

	// Returns the Transform data of this Node's parent
	virtual Transform GetParentTransform() const;

	// Recalculate the Model Matrix and update the transforms of all children. Again this should probably only be on Node3D
	virtual void UpdateTransform();

	// Get the SceneTree this node is currently inside (if one exists)
	SceneTree* GetTree();

	// If we are in a SceneTree, get the root node. Otherwise walk the tree until we find the outermost node
	Node* GetRootNode();

	// Same as GetRootNode, but automatically casted
	template<typename T>
	T* GetRootNode();

	// Get the node who owns this node, if one exists
	Node* GetParent() const { return Parent.Get(); }

	// Walk the tree upwards until we find a Node with the matching type, if one exists.
	template<typename T>
	T* GetAncestorOfType();

	// Walks the tree upwards to check if this node has otherNode as an ancestor
	bool IsDescendantOf(Node* otherNode);

	// Walks the tree downwards to check if we are an ancestor or otherNode
	bool IsAncestorOf(Node* otherNode) const;

	// Tells the Draw function to skip this node and its children
	void SetHidden(bool newIsHidden);

	// Is this flagged as Hidden (will not be rendered, children will not be rendered)
	bool IsHidden();

	// Is this node currently in a Scene Tree
	bool IsInTree();

	// Has Ready been called
	bool IsReady();

	// Removes this node from its parent and returns the owning reference. Not caching the return value will cause this node to be destroyed
	std::unique_ptr<Node> RemoveFromParent();

	// Removes this node from its current parent and adds it to a new one. This does not check if it is a valid move
	// IE Reparenting on to a child node could break everything
	void Reparent(Node* newParent);

	// Allows drawing to the Inspector panel, if one exists and we are running in an editor
	virtual void DrawInspectorWidget();
	
	// Instantiate a node and add it to the list of children
	template <typename T, typename ... TArgs>
	T* AddChild(const TArgs&... args);

	// Add an existing node to the list of children
	Node* AddChild(std::unique_ptr<Node> node);

	// Gets a snapshot of the node's children. These pointers could be invalidated at any time
	template<typename T = Node>
	std::vector<T*> GetChildrenOfType() const;

	// Gets a snapshot of the node's children. These pointers could be invalidated at any time
	std::vector<Node*> GetChildren() const;

	// Gets a snapshot of the node's children. These pointers are slower but will be set to null if the object is invalidated
	std::vector<ObjectRef<Node>> GetChildrenSafe() const;

	template<typename Functor>
	void ForEachChild(Functor functor);

	template<typename ChildClass, typename Functor>
	void ForEachChildSafe(Functor functor);

	template<typename NodeType, typename Functor>
	void ForEachChildOfType(Functor functor) const;

	template<typename NodeType, typename Functor>
	void ForEachChildOfType(Functor functor);

	// Retrieve the first child of type T if one exists
	template<typename T>
	T* GetChild() const;

	// Set this node's name
	void SetName(const std::string& name) { Name = name; }

	// Get this Node's name
	const std::string& GetName() const { return Name; }



	virtual void Serialize(nlohmann::json& archive) override;
	virtual void Deserialize(nlohmann::json& archive) override;
	
protected:
	// Tree walking for DrawGUI. // TODO: remove and do it in the caller
	virtual void DrawGUIInternal();

private:
	std::string Name = "Node";
	bool isHidden = false;

	std::vector<std::unique_ptr<Node>> Children; // All owned child nodes
	ObjectRef<Node> Parent; // Parent node. If nullptr, assume this is node is the SceneRoot
	bool isInTree = false; // Is currently in a tree
	bool isReady = false; // Has ready been called
	SceneTree* tree = nullptr; // Owning Tree
};

template<typename NodeType, typename Functor>
void Node::ForEachChildOfType(Functor functor) const {
    for (int i = 0; i < Children.size(); i++)
    {
        if (Children.at(i) == nullptr) continue;
        NodeType* n = dynamic_cast<NodeType*>(Children[i].get());
        if (n)
        {
            functor(n);
        }
    }
}


template<typename T>
std::unique_ptr<T> Node::InitializeNode(T *raw, const std::string &name) {
	raw->SetName(name);
	raw->Init();

	std::unique_ptr<T> n;
	n.reset(raw);
	return n;
}

template<typename T>
std::unique_ptr<T> Node::NewNode(const std::string &name) {
	T* node = new T();
	return InitializeNode(node, name);
}

template<typename T>
T * Node::GetRootNode() {
	auto root = GetRootNode();
	return Object::Cast<T>(root);
}

template<typename T>
T * Node::GetAncestorOfType() {
	if (Parent)
	{
		if (T* ancestor = Object::Cast<T>(Parent.Get()))
		{
			return ancestor;
		}
		return Parent->GetAncestorOfType<T>();
	}
	return nullptr;
}

template<typename T, typename ... TArgs>
T * Node::AddChild(const TArgs &...args) {
	T* created = static_cast<T*>(Children.emplace_back(NewNode<T>(args...)).get());
	created->Parent = this;
	if (tree)
	{
		tree->RegisterNode(created);
		if (isReady) tree->ReadyNode(created);
	}
	UpdateTransform();
	return created;
}

template<typename T>
std::vector<T *> Node::GetChildrenOfType() const {
	std::vector<T*> children;
	for (auto& child : Children)
	{
		if (child == nullptr) continue;
		T* childNode = dynamic_cast<T*>(child.get());
		if (childNode)
		{
			children.push_back(childNode);
		}
	}

	return children;
}

template<typename Functor>
void Node::ForEachChild(Functor functor) {
	for (int i = 0; i < Children.size(); i++)
	{
		if (Children.at(i) == nullptr) continue;
		functor(Children.at(i).get());
	}
}

template<typename ChildClass, typename Functor>
void Node::ForEachChildSafe(Functor functor) {
	std::vector<ObjectRef<ChildClass>> safe_children;
	for (int i = 0; i < Children.size(); i++) {
		//if (!i <= Children.size()) break;
		if (auto ptr = Children.at(i).get()) {
			safe_children.emplace_back(ObjectRef<ChildClass>(ptr));
		}
	}

	for (auto& safe : safe_children)
	{
		if (!safe.IsValid()) continue;
		functor(safe);
	}
}

template<typename NodeType, typename Functor>
void Node::ForEachChildOfType(Functor functor) {
	for (int i = 0; i < Children.size(); i++)
	{
		if (Children.at(i) == nullptr) continue;
		NodeType* n = dynamic_cast<NodeType*>(Children[i].get());
		if (n)
		{
			functor(n);
		}
	}
}

template<typename T>
T * Node::GetChild() const {
	auto children = GetChildrenOfType<T>();
	if (!children.empty())
		return children.front();
	return nullptr;
}




