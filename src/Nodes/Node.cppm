//
// Created by Vinnie on 22/02/2025.
module;
#pragma once

#include <memory>
#include <string>
#include <vector>

//#include "Transform.cppm"
//#include "Core/Object.cppm"
#include "ReflectionMacros.h"
//#include "Core/ClassDB.cppm"

export module Engine.Node;
export import Engine.Object;
export import Engine.Reflection.ClassDB;
export import Engine.Transform;
export import Engine.Reflection.Class;
import Engine.Reflection.Class;
import Engine.Object.ObservableDtor;
import Engine.SceneTree;

export class RenderVisitor;

export class Node : public Object {
public:
	CRAB_CLASS(Node, Object)
	CLASS_FLAG(EditorVisible)
	BEGIN_PROPERTIES
		ADD_PROPERTY("Name", Name)
		ADD_PROPERTY("Hidden", isHidden)
	END_PROPERTIES

protected:
	friend class Application;
	friend class NodeWindow;
	friend class NodeImGUIContextWindow;
	friend class SceneTree;
	friend class Object;


	
	// Runs every frame before the Update function, with the latest controller input data
	//virtual void ProcessInput(const Controller::Input::ControllerContext& PadData, int padIndex) {};

	// Runs every frame
	virtual void Update(float dt) {}

	virtual void DrawGUI() {};

public:


public:
	virtual ~Node() override;

protected:
	Node() {}

public:
	template<typename T = Node>
	static std::unique_ptr<T> InitializeNode(T* raw, const std::string& name) {
		raw->SetName(name);
		raw->Init();

		std::unique_ptr<T> n;
		n.reset(raw);
		return n;
	}

	template<typename T = Node>
	static std::unique_ptr<T> NewNode(const std::string& name = "Node")
	{
		T* node = new T();
		return InitializeNode(node, name);
	};
	//static std::unique_ptr<Node> NewNode(Node* raw, const std::string& name);
	//static std::unique_ptr<Node> MakeNode()
	std::unique_ptr<Node> Duplicate();

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

	SceneTree* GetTree() {return tree;}
	
	// Walk the tree to find the outermost Node (Generally the SceneRoot)
	Node* GetRootNode();
	template<typename T>
	T* GetRootNode()
	{
		auto root = GetRootNode();
		return dynamic_cast<T*>(root);
	};

	Node* GetParent() { return Parent; }

	template<typename T>
	T* GetAncestorOfType(){
		if (Parent)
		{
			if (T* ancestor = dynamic_cast<T*>(Parent))
			{
				return ancestor;
			}
			return Parent->GetAncestorOfType<T>();
		}
		return nullptr;
	};

	template<typename T>
	bool IsA()
	{
		return dynamic_cast<T*>(this) != nullptr;
	};

	bool IsDescendantOf(Node* otherNode)
	{
		if (!otherNode) return false;
		return (otherNode->IsAncestorOf(this));
	};
	bool IsAncestorOf(Node* otherNode)
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
	};
	
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
	template <typename T, typename ... TArgs>
	T* AddChild(const TArgs&... args)
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

	Node* AddChild(std::unique_ptr<Node> node);

	// Gets a snapshot of the node's children. These pointers could be invalidated at any time
	// For iteration, prefer to use ForEachChild()
	template<typename T = Node>
	std::vector<T*> GetChildrenOfType() const
	{
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

	// Gets a snapshot of the node's children. These pointers could be invalidated at any time
	// For iteration, prefer to use ForEachChild()
	std::vector<Node*> GetChildren() const
	{
		std::vector<Node*> children;
		for (auto& child : Children)
		{
			if (child == nullptr) continue;
			children.push_back(child.get());
		}
		return children;
	}

	template<typename Functor>
	void ForEachChild(Functor functor)
	{
		for (int i = 0; i < Children.size(); i++)
		{
			if (Children.at(i) == nullptr) continue;
			functor(Children.at(i).get());
		}
	}

	template<typename NodeType, typename Functor>
	void ForEachChildOfType(Functor functor) const
	{
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

	template<typename NodeType, typename Functor>
	void ForEachChildOfType(Functor functor)
	{
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
	T* GetChild() const
	{
		auto children = GetChildrenOfType<T>();
		if (!children.empty())
			return children.front();
		return nullptr;
	}

	void SetName(const std::string& name) { Name = name; }
	const std::string& GetName() const { return Name; }

	std::string Name = "Node";
	bool isHidden = false;

	virtual void Serialize(nlohmann::json& archive) override;
	virtual void Deserialize(nlohmann::json& archive) override;
	
protected:
	
	// Hidden flag

	// Child nodes
private:
	std::vector<std::unique_ptr<Node>> Children;

protected:
	// Parent node. If nullptr, assume this is SceneRoot
	Node* Parent = nullptr;

	
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






