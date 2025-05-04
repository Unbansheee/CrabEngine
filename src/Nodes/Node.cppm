//
// Created by Vinnie on 22/02/2025.
module;
#pragma once

#include "ReflectionMacros.h"

export module Engine.Node;
export import Engine.Object;
export import Engine.Transform;
import Engine.Object.Ref;
import std;
import Engine.SceneTree;
export import Engine.Input;

export class RenderVisitor;
export class Renderer;

export class Node : public Object {
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
	static void NativeGetName(ThisClass* ctx, wchar_t* outString) {
		auto name = ctx->GetName();
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
		std::wstring wideClass = converter.from_bytes(name);

		std::wmemcpy(outString, wideClass.c_str(), name.size());
		outString[name.size()] = L'\0'; // Null-terminate
	};
	BIND_STATIC_METHOD(const wchar_t*, NativeGetName);
	BIND_METHOD_PARAMS(InputResult, HandleInput, InputEventInterop event, (event))

protected:
	friend class Application;
	friend class NodeWindow;
	friend class NodeImGUIContextWindow;
	friend class SceneTree;
	friend class Object;

	// Runs every frame
	virtual void Update(float dt) {

	}

	virtual void DrawGUI()
	{
	};

	virtual InputResult HandleInput(const InputEvent& event) { return InputResult::Ignored; }

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

	static std::unique_ptr<Node> NewNode(const ClassType& classType, const std::string& name = "Node")
	{
		Node* node = static_cast<Node*>(classType.Initializer());
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

	virtual void Render(Renderer& renderer);

	// Returns the Transform data of this Node
	virtual Transform GetTransform() const;

	// Returns the Transform data of this Node's parent
	virtual Transform GetParentTransform() const;

	// Recalculate the Model Matrix and update the transforms of all children
	virtual void UpdateTransform();

	// Walk the tree to the root node, and retrieve its Context
	//Context& GetContext();

	SceneTree* GetTree();

	// Walk the tree to find the outermost Node (Generally the SceneRoot)
	Node* GetRootNode();

	template<typename T>
	T* GetRootNode()
	{
		auto root = GetRootNode();
		return dynamic_cast<T*>(root);
	};

	Node* GetParent() { return Parent.Get(); }

	template<typename T>
	T* GetAncestorOfType(){
		if (Parent)
		{
			if (T* ancestor = dynamic_cast<T*>(Parent.Get()))
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

	std::vector<ObjectRef<Node>> GetChildrenSafe() const
	{
		std::vector<ObjectRef<Node>> children;
		for (auto& child : Children)
		{
			if (child == nullptr) continue;
			children.emplace_back(child.get());
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

	template<typename ChildClass, typename Functor>
	void ForEachChildSafe(Functor functor)
		{
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
private:
	std::vector<std::unique_ptr<Node>> Children;

protected:
	// Parent node. If nullptr, assume this is SceneRoot
	ObjectRef<Node> Parent;

	virtual void DrawGUIInternal();
	
	bool isInTree = false;
	bool isReady = false;
private:
	SceneTree* tree = nullptr;



};






