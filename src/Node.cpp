//
// Created by Vinnie on 22/02/2025.
//

#include "Node.h"
#include "imgui/imgui.h"
#include "imgui/misc/cpp/imgui_stdlib.h"

void Node::DrawInspectorWidget()
{
	char namebuf[64];
	ImGui::InputText("Name", &Name);
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

	int i = 0;
	auto it = std::find_if(Parent->Children.begin(), Parent->Children.end(), [this](auto& a)
	{
		return a.get() == this;
	});
	if (it != Parent->Children.end())
	{
		std::unique_ptr<Node> n = std::move(*it);
		Parent->Children.erase(it);
		return n;
	}

	return nullptr;
	/*
	for (std::unique_ptr<Node>& item : Parent->Children)
	{
		if (item.get() == this)
		{
			break;
		}
		i++;
	}

	std::unique_ptr<Node> n = std::move(Parent->Children.at(i));
	Parent->Children.erase(Parent->Children.begin() + i);
	return n;
	*/
}

void Node::UpdateNodeInternal(float dt)
{

	// Depth first walk
	for (const auto& i : Children)
	{
		if (!i) continue;
		i->UpdateNodeInternal(dt);
	}

	Update(dt);
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

void Node::BeginInternal()
{
	UpdateTransform();

	// Depth first walk
	for (const auto& i : Children)
	{
		if (!i) continue;
		if (i->hasBegun) continue;

		i->BeginInternal();
	}

	if (hasBegun) return;

	hasBegun = true;
	Begin();
}