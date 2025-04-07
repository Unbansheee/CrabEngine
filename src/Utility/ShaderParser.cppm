//
// Created by Vinnie on 31/03/2025.
//

export module Engine.ShaderParser;
import std;
import Engine.WGPU;
import Engine.Resource.Material;
import Engine.MaterialProperties;


export class ShaderParser {
public:
    ShaderMetadata Parse(const std::string& source);
    const std::string& GetProcessedSource() {return ProcessedSource;};

private:
    bool ProcessPropertiesLine(const std::string &line, ShaderMetadata &metadata, std::stringstream& propertiesStruct);
    void ProcessFunctionLine(const std::string& scopeName, int& counter, const std::string& line, std::stringstream& output);
    void ProcessBaseShaderTypeLine(const std::string& line, std::string& output);
    
    std::string ProcessedSource{};

    static inline std::unordered_map<std::string, std::string> ShaderSources = {{"StandardMaterial", ENGINE_RESOURCE_DIR"/standard_material_2.wgsl"}};     
};

export class ShaderProcessor {
public:
    std::string ProcessIncludes(const std::string& source);
    std::string ExtractMetadata(const std::string& source);
};

export class BindingBuilder {
public:
    void AddUniformBuffer(uint32_t size);
    void AddTexture();

    wgpu::BindGroupLayout GetLayout();
    wgpu::BindGroup GetGroup();
};

export class UniformBufferBuilder {
public:
    void AddField(const std::string& name, MaterialPropertyType type);
    wgpu::Buffer CreateBuffer(wgpu::Device device);
};