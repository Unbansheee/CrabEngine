module Engine.Resource;
import Engine.Resource.ResourceManager;
import Engine.Resource.Texture;

// Resource map. Keeps resources alive while they are referenced in C#. When a C# Resource object dies it will be released from here
std::unordered_map<Resource*, std::shared_ptr<Resource>> g_ResourceLifetimeMap;

void Resource::Serialize(nlohmann::json &archive) { Object::Serialize(archive); }

void Resource::Deserialize(nlohmann::json &archive) { Object::Deserialize(archive); }

Resource::~Resource() = default;

void Resource::LoadData() { loaded = true; }

bool Resource::IsLoaded() const {return loaded;}

void Resource::LoadIfRequired() const { if (!IsLoaded()) const_cast<Resource*>(this)->LoadData(); }

wgpu::raii::TextureView Resource::GetThumbnail() {
    if (!DefaultResourceThumbnail) {
        DefaultResourceThumbnail = ResourceManager::Load<TextureResource>("/engine/Textures/T_DefaultResourceThumbnail.png");
    }
    return DefaultResourceThumbnail->GetInternalTextureView();
}

const std::string & Resource::GetName() const {return name;}

bool Resource::IsSourceImported() const { return !bIsInline; }

bool Resource::IsInline() const { return bIsInline; }

const std::string & Resource::GetSourcePath() { return sourcePath; }

const std::string & Resource::GetAbsolutePath() {
    if (absolutePath.empty()) {
        absolutePath = Filesystem::AbsolutePath(sourcePath);
    }
    return absolutePath;
}

const std::shared_ptr<ResourceMetadata> & Resource::GetImportSettings() { return importSettings; }

void Resource::ReleaseResource(Resource* res) {
    g_ResourceLifetimeMap.erase(res);
}

Resource * Resource::LoadResource(const char *path) {
    auto res = ResourceManager::Load(path);
    if (!res) return nullptr;

    g_ResourceLifetimeMap.insert({res.get(), res});
    return res.get();
}
