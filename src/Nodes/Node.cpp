//
// Created by Vinnie on 22/02/2025.
//

#include "Node.h"

#include "Core/ClassDB.h"
#include "Core/SceneTree.h"
#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

REGISTER_CLASS(Node)

void Node::DrawInspectorWidget()
{
	//for (auto& prop : GetProperties())
	{
		//prop.DrawProperty();		
	}
}

Node* Node::AddChild(std::unique_ptr<Node> node)
{
	Children.emplace_back(std::move(node));
	auto& n = Children.back();
	n->Parent = this;

	// if node is not already in the tree but we are, register it to the tree
	if (tree)
	{
		if (!n->tree)
		{
			tree->RegisterNode(n.get());
		}
			
		if (!n->isReady && isReady) tree->ReadyNode(n.get());
	}
		
	UpdateTransform();
	return n.get();
}




Node::~Node()
{
	//if (tree) tree->UnregisterNode(this);
}

Transform Node::GetTransform() const
{
	return Transform::identity();
}

Transform Node::GetParentTransform() const
{
	if (Parent)
	{
		return Parent->GetTransform();
	}
	else
	{
		return Transform::identity();
	}
}

void Node::UpdateTransform()
{
	for (auto& child : Children)
	{
		child->UpdateTransform();
	}
}

void Node::Render(RenderVisitor& Visitor)
{
	// Depth first walk
	for (const auto& i : Children)
	{
		if (!i) continue;
		if (i->IsHidden()) continue;
		i->Render(Visitor);
	}
}

/*
void Node::GatherDrawCommands(std::vector<DrawCommand> &Commands) const {
	for (auto& child : Children)
	{
		if (!child) continue;
		if (child->IsHidden()) continue;
		child->GatherDrawCommands(Commands);
	}
}
*/

/*
std::vector<DrawCommand> Node::CreateDrawCommand(Context& context) const
{
	std::vector<DrawCommand> buffer;
	for (auto& child : Children)
	{
		if (!child) continue;
		if (child->IsHidden()) continue;
		std::vector<DrawCommand> childBuffer = child->CreateDrawCommand(context);
		buffer.insert(buffer.end(), childBuffer.begin(), childBuffer.end());
	}
	return buffer;
}
*/

/*
Context& Node::GetContext()
{
	return *GetRootNode()->OwningContext;
}
*/

Node* Node::GetRootNode()
{
	if (Parent)
		return Parent->GetRootNode();
	else
		return this;
}

/*
Application* Node::GetApplication()
{
	if (ApplicationContext) return ApplicationContext;
	if (Parent) return Parent->GetApplication();
	return nullptr;
}
*/

void Node::SetHidden(bool newIsHidden)
{
	this->isHidden = isHidden;
}

std::unique_ptr<Node> Node::RemoveFromParent()
{
	if (!Parent) return nullptr;

	auto it = std::find_if(Parent->Children.begin(), Parent->Children.end(), [this](auto& a)
	{
		return a.get() == this;
	});
	if (it != Parent->Children.end())
	{
		std::unique_ptr<Node> n = std::move(*it);
		Parent->Children.erase(it);
		
		if (n->tree) n->tree->UnregisterNode(n.get());
		
		return n;
	}
	
	return nullptr;
}

void Node::Reparent(Node* newParent)
{
	if (!Parent) return;
	assert(newParent);

	auto it = std::find_if(Parent->Children.begin(), Parent->Children.end(), [this](auto& a)
	{
		return a.get() == this;
	});
	
	if (it != Parent->Children.end())
	{
		std::unique_ptr<Node> n = std::move(*it);
		Parent->Children.erase(it);
		auto t = n.get();
		newParent->Children.push_back(std::move(n));

		// Moving to a node thats not in the tree, unregister
		if (t->tree && !newParent->tree) t->tree->UnregisterNode(t);

		// Moving to a node thats in the tree, register
		if (!t->tree && newParent->tree) newParent->tree->RegisterNode(t);
	}
}

void Node::DrawGUIInternal()
{
	DrawGUI();

	// Depth first walk
	for (const auto& i : Children)
	{
		if (!i) continue;
		i->DrawGUIInternal();
	}
}

/*
void Node::ProcessInputInternal(const Controller::Input::ControllerContext& PadData, int padIndex)
{
	ProcessInput(PadData, padIndex);

	// Depth first walk
	for (const auto& i : Children)
	{
		if (!i) continue;
		i->ProcessInputInternal(PadData, padIndex);
	}
}
*/

