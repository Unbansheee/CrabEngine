//
// Created by Vinnie on 22/02/2025.

#pragma once
#include <memory>
#include <vector>
#include "IDrawable.h"
#include "Transform.h"

class Node : public IDrawable {

protected:
	friend class Application;

	// Runs when this node is instantiated
	virtual void Begin() {}

	// Runs every frame before the Update function, with the latest controller input data
	//virtual void ProcessInput(const Controller::Input::ControllerContext& PadData, int padIndex) {};

	// Runs every frame
	virtual void Update(float dt) {}

public:
	virtual ~Node()
	{
	};

	// Returns the Transform data of this Node
	virtual Transform GetTransform() const;

	// Returns the Transform data of this Node's parent
	virtual Transform GetParentTransform() const;

	// Recalculate the Model Matrix and update the transforms of all children
	virtual void UpdateTransform();

	// IDrawable
	virtual void GatherDrawCommands(std::vector<DrawCommand>& Commands) const override;
	// ~IDrawable

	// Walk the tree to the root node, and retrieve its Context
	//Context& GetContext();

	// Walk the tree to find the outermost Node (Generally the SceneRoot)
	Node* GetRootNode();

	// Walk the tree to the root node, and retrieve its Application
	//Application* GetApplication();

	// Tells the Draw function to skip this node and its children
	void SetHidden(bool newIsHidden);

	// Is this flagged as Hidden
	bool IsHidden() { return isHidden; }

	std::unique_ptr<Node> RemoveFromParent();

	// Instantiate a node and add it to the list of children
	template<typename T, typename... TArgs>
	T* AddChild(const TArgs&... args)
	{
		T* created = static_cast<T*>(Children.emplace_back(std::make_unique<T>(args...)).get());
		created->Parent = this;
		UpdateTransform();
		if (hasBegun == true)
			created->BeginInternal();
		return created;
	}

	Node* AddChild(std::unique_ptr<Node> node, bool skipBegin = false)
	{
		Children.emplace_back(std::move(node));
		auto& n = Children.back();
		n->Parent = this;
		UpdateTransform();
		if (n->hasBegun == false && skipBegin == false && hasBegun == true)
			n->BeginInternal();
		return n.get();
	}

	template<typename T>
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
protected:
	std::string Name = "Node";

	// Child nodes
	std::vector<std::unique_ptr<Node>> Children;

	// Parent node. If nullptr, assume this is SceneRoot
	Node* Parent = nullptr;

	// Call Update() and update children
	void UpdateNodeInternal(float dt);

	// Call ProcessInput() and update children inputs
	//void ProcessInputInternal(const Controller::Input::ControllerContext& PadData, int padIndex);

	void BeginInternal();

	// Hidden flag
	bool isHidden = false;

	bool hasBegun = false;
private:
	// RenderContext that 'owns' this. Only valid if this is the Root node. Should work on a better way to do this
	//Context* OwningContext = nullptr;
	//Application* ApplicationContext = nullptr;
};


