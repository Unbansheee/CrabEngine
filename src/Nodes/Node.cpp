//
// Created by Vinnie on 22/02/2025.
//

module;

module Engine.Node;
import Engine.Filesystem;

void Node::DrawInspectorWidget()
{
}

Node* Node::AddChild(std::unique_ptr<Node> node)
{
	Children.emplace_back(std::move(node));
	auto& n = Children.back();
	n->Parent = this;

	// if node is not already in the tree but we are, register it to the tree
	if (tree && isInTree)
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

std::vector<Node *> Node::GetChildren() const {
	std::vector<Node*> children;
	for (auto& child : Children)
	{
		if (child == nullptr) continue;
		children.push_back(child.get());
	}
	return children;
}

std::vector<ObjectRef<Node>> Node::GetChildrenSafe() const {
	std::vector<ObjectRef<Node>> children;
	for (auto& child : Children)
	{
		if (child == nullptr) continue;
		children.emplace_back(child.get());
	}
	return children;
}

void Node::NativeGetName(ThisClass *ctx, wchar_t *outString) {
	auto name = ctx->GetName();
	std::wstring wideClass = Filesystem::StringToWString(name);

	std::wmemcpy(outString, wideClass.c_str(), name.size());
	outString[name.size()] = L'\0'; // Null-terminate
}

Node::~Node()
{
}

std::unique_ptr<Node> Node::Duplicate() {
	auto newnode = static_cast<Node*>(GetStaticClassFromThis().Initializer());
	for (auto prop : GetPropertiesFromThis()) {
		if (prop.name == "id") continue;
		prop.setVariant(newnode, prop.getVariant(this));
	}
	std::unique_ptr<Node> n = InitializeNode(newnode, GetName());
	ForEachChild([&n](Node* child) {
		if (child->HasFlag(ObjectFlags::Transient)) return;
		n->AddChild(child->Duplicate());
	});

	return n;
}

void Node::Render(Renderer& renderer) {
}


Transform Node::GetTransform() const
{
	return GetParentTransform();
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
	ForEachChild([](Node* child)
	{
		child->UpdateTransform();
	});
}

SceneTree * Node::GetTree() {return tree;}

Node* Node::GetRootNode()
{
	if (tree) return tree->GetRoot();

	else if (Parent)
		return Parent->GetRootNode();
	else
		return this;
}

bool Node::IsDescendantOf(Node *otherNode) {
	if (!otherNode) return false;
	return (otherNode->IsAncestorOf(this));
}

bool Node::IsAncestorOf(Node *otherNode) const {
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
		std::erase_if(Parent->Children, [](std::unique_ptr<Node>& n){ return n == nullptr; });
		
		if (n->tree && isInTree) n->tree->UnregisterNode(n.get());
		
		return n;
	}
	
	return nullptr;
}

void Node::Reparent(Node* newParent)
{
	if (!Parent) return;
	//assert(newParent);

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

void Node::Serialize(nlohmann::json& archive)
{
	Object::Serialize(archive);
	auto childArray = nlohmann::json::array();
	ForEachChild([&childArray](Node* child)
	{
		if (!child->HasFlag(ObjectFlags::Transient)) {
			auto childData = nlohmann::json::object();
			child->Serialize(childData);
			childArray.push_back(childData);
		}
	});
	archive["children"] = childArray;
}

void Node::Deserialize(nlohmann::json& archive)
{
	Object::Deserialize(archive);
	
	for (auto& childJson : archive["children"])
	{
		auto childType = childJson.at("class").get<std::string>();
		auto classType = ClassDB::Get().GetClassByName(childType);
		if (!classType) {
			std::cerr << "Could not instantiate Node with ClassType " << childType << std::endl;
			continue;
		}
		if (classType->Initializer) {
			Object* n = classType->Initializer();
			Node* node = dynamic_cast<Node*>(n);
			auto instance = Node::InitializeNode(node, classType->Name.string());
			instance->Deserialize(childJson);
			AddChild(std::move(instance));
		}
	}
}

void Node::DrawGUIInternal()
{
	DrawGUI();

	ForEachChildSafe<Node>([](ObjectRef<Node>& Child)
	{
		Child->DrawGUIInternal();
	});
}


