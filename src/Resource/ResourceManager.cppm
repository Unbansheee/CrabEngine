//
// Created by Vinnie on 21/02/2025.
//
module;

export module Engine.Resource.ResourceManager;
import Engine.Resource;
import Engine.Resource.Importer.ImportSettings;
import Engine.WGPU;
import Engine.Types;
import Engine.MaterialProperties;

export struct MeshVertex;
export class ImportManager;

struct ResourceCache {
    std::unordered_map<std::string, std::shared_ptr<Resource>> pathCache;
    std::unordered_map<UID, std::shared_ptr<Resource>> idCache;

    void AddResource(std::shared_ptr<Resource> res) {
        pathCache.insert({res->GetSourcePath(), res});
        idCache.insert({res->GetID(), res});
    }

    std::shared_ptr<Resource> FetchResourceByPath(const std::string& path) {
        if (pathCache.contains(path)) {
            return pathCache.at(path);
        }
        return nullptr;
    }

    std::shared_ptr<Resource> FetchResourceByID(const UID& id) {
        if (idCache.contains(id)) {
            return idCache.at(id);
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<Resource>> GetAllResources()
    {
        std::vector<std::shared_ptr<Resource>> r;
        r.reserve(idCache.size());

        for (auto kv : pathCache)
        {
            r.push_back(kv.second);
        }
        return r;
    }

};


export struct ShaderModule {
    wgpu::ShaderModule Module = nullptr;
    ShaderMetadata Metadata;
};

export class ResourceManager {
public:
    static bool IsSourceFile(const std::filesystem::path& path);

    template<typename T>
    static std::shared_ptr<T> Load(const std::filesystem::path& path) {
            return std::static_pointer_cast<T>(Load(path));
    }

    template<typename T>
    static std::shared_ptr<T> FindByID(const UID& id) {
            return std::static_pointer_cast<T>(FindByID(id));
        }

    static std::shared_ptr<Resource> Load(const std::filesystem::path& path);
    static std::shared_ptr<Resource> FindByID(const UID& id);

    static void PollResourcesForChanges();

    static void ReloadResource(const std::shared_ptr<Resource>& resource);
    static bool IsResourceLoaded(const std::filesystem::path& path);
    static void SaveToFile(const std::filesystem::path& path, nlohmann::json& json);
    static void SaveImportSettings(const std::filesystem::path& sourcePath, const std::shared_ptr<ResourceMetadata>& importSettings);
    static void SaveResource(const std::shared_ptr<Resource>& resource, 
                      const std::filesystem::path& path = {}) {
        auto savePath = path.empty() ? resource->GetSourcePath() : path;

        if (savePath.extension() == ".res") {
            nlohmann::json j;
            resource->Serialize(j);
            SaveToFile(savePath, j);
        }
        if (auto importSettings = resource->GetImportSettings()) {
            importSettings->ResourceID = resource->GetID();
            SaveImportSettings(savePath, importSettings);
        }

        resource->OnResourceSaved.invoke();
    }

    static std::vector<std::shared_ptr<Resource>> GetAllResources();
    
private:
    inline static std::mutex cacheMutex;
    inline static ResourceCache cache;

    //TODO: Remove
public:
    static bool loadGeometryFromObj(const std::filesystem::path& path,
    std::vector<MeshVertex> &vertexData);

    static wgpu::Texture loadTexture(const std::filesystem::path& path, wgpu::Device device, wgpu::TextureView* pTextureView);
    static wgpu::Texture loadTexture(wgpu::Device device, int width, int height, int channels, unsigned char* pixelData, wgpu::TextureView* pTextureView);
    
    static ShaderModule loadShaderModule(
    const std::filesystem::path& path,
    wgpu::Device device);

    static wgpu::ShaderModule loadComputeShaderModule(const std::filesystem::path &path, wgpu::Device device, std::unordered_map<std::string, std::string>& formats);


    static void populateTextureFrameAttributes(std::vector<MeshVertex> &vertexData, optional_ref<const std::vector<uint16_t>> indices = std::nullopt);
    static void writeMipMaps(
        wgpu::Device device,
        wgpu::Texture texture,
        wgpu::Extent3D textureSize,
    [[maybe_unused]] uint32_t mipLevelCount, // not used yet
    const unsigned char* pixelData);

private:
    static glm::mat3x3 computeTBN(const MeshVertex corners[3], const glm::vec3& expectedN);
};


