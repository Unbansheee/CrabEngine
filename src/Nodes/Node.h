//
// Created by Vinnie on 22/02/2025.

#pragma once
#include <memory>
#include <string>
#include <vector>

#include "Utility/ObservableDtor.h"
#include "Transform.h"
#include "Core/Object.h"
#include "Core/SceneTree.h"
#include "Core/UID.h"

class RenderVisitor;

class Node : public Object, public observable_dtor {
protected:
	friend class Application;
	friend class NodeWindow;
	friend class NodeImGUIContextWindow;
	friend class SceneTree;

	// Runs every frame before the Update function, with the latest controller input data
	//virtual void ProcessInput(const Controller::Input::ControllerContext& PadData, int padIndex) {};

	// Runs every frame
	virtual void Update(float dt) {}

	virtual void DrawGUI() {};

public:
	BEGIN_PROPERTIES(Object)
		ADD_PROPERTY(Node, "Name", Name)
		ADD_PROPERTY_FLAGS(Node, "Hidden", isHidden, Property::Flags::Transient)
	END_PROPERTIES

public:
	virtual ~Node() override;

protected:
	Node() {}

public:
	template<typename T = Node>
	static std::unique_ptr<T> NewNode(const std::string& name = "Node");

	// Called when the node enters the SceneTree
	virtual void EnterTree() {};
	// Called when the node exits the SceneTree
	virtual void ExitTree() {};
	// Called when all of the child nodes are ready
	virtual void Ready() {};
	// Called upon initialization
	virtual void Init() {};
	
	// Returns the Transform data of this Node
	virtual Transform GetTransform() const;

	// Returns the Transform data of this Node's parent
	virtual Transform GetParentTransform() const;

	// Recalculate the Model Matrix and update the transforms of all children
	virtual void UpdateTransform();

	// IDrawable
	virtual void Render(RenderVisitor& Visitor);
	// ~IDrawable

	// Walk the tree to the root node, and retrieve its Context
	//Context& GetContext();

	// Walk the tree to find the outermost Node (Generally the SceneRoot)
	Node* GetRootNode();
	template<typename T>
	T* GetRootNode();

	Node* GetParent() { return Parent; }

	template<typename T>
	T* GetAncestorOfType();

	template<typename T>
	bool IsA();

	bool IsDescendantOf(Node* otherNode);
	bool IsAncestorOf(Node* otherNode);
	
	// Walk the tree to the root node, and retrieve its Application
	//Application* GetApplication();

	// Tells the Draw function to skip this node and its children
	void SetHidden(bool newIsHidden);

	// Is this flagged as Hidden
	bool IsHidden() { return isHidden; }

	std::unique_ptr<Node> RemoveFromParent();
	void Reparent(Node* newParent);

	virtual void DrawInspectorWidget();
	
	// Instantiate a node and add it to the list of children
	template<typename T, typename... TArgs>
	T* AddChild(const TArgs&... args);

	Node* AddChild(std::unique_ptr<Node> node);

	template<typename T = Node>
	std::vector<T*> GetChildren() const
	{
		std::vector<T*> children;
		for (auto& child : Children)
		{
			T* childNode = dynamic_cast<T*>(child.get());
			if (childNode)
			{
				children.push_back(childNode);
			}
		}

		return children;
	}

	template<typename T>
	T* GetChild() const
	{
		for (auto& child : Children)
		{
			if (T* childNode = static_cast<T*>(child.get()))
			{
				return childNode;
			}
		}
		return nullptr;
	}

	void SetName(const std::string& name) { Name = name; }
	const std::string& GetName() const { return Name; }

	std::string Name = "Node";
	bool isHidden = false;

protected:
	
	// Hidden flag

	// Child nodes
	std::vector<std::unique_ptr<Node>> Children;

	// Parent node. If nullptr, assume this is SceneRoot
	Node* Parent = nullptr;

	UID id ;
	
	virtual void DrawGUIInternal();
	
	// Call ProcessInput() and update children inputs
	//void ProcessInputInternal(const Controller::Input::ControllerContext& PadData, int padIndex);
	
	bool isInTree = false;
	bool isReady = false;
private:
	SceneTree* tree;
	// RenderContext that 'owns' this. Only valid if this is the Root node. Should work on a better way to do this
	//Context* OwningContext = nullptr;
	//Application* ApplicationContext = nullptr;
};

inline bool Node::IsDescendantOf(Node* otherNode)
{
	if (!otherNode) return false;
	return (otherNode->IsAncestorOf(this));
}

inline bool Node::IsAncestorOf(Node* otherNode)
{
	if (!otherNode) return false;
	Node* parent = otherNode->GetParent();
	while (parent != nullptr)
	{
		if (parent == this)
		{
			return true;
		}
		parent = parent->GetParent();
	}
	return false;
}

template <typename T>
std::unique_ptr<T> Node::NewNode(const std::string& name)
{
	T* node = new T();
	node->SetName(name);
	node->Init();

	std::unique_ptr<T> ptr;
	ptr.reset(node);
	return ptr;
}

template <typename T>
T* Node::GetRootNode()
{
	auto root = GetRootNode();
	return dynamic_cast<T*>(root);
}

template <typename T>
T* Node::GetAncestorOfType()
{
	if (Parent)
	{
		if (T* ancestor = dynamic_cast<T*>(Parent))
		{
			return ancestor;
		}
		return Parent->GetAncestorOfType<T>();
	}
	return nullptr;
}

template <typename T>
bool Node::IsA()
{
	return dynamic_cast<T*>(this) != nullptr;
}

template <typename T, typename ... TArgs>
T* Node::AddChild(const TArgs&... args)
{
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
