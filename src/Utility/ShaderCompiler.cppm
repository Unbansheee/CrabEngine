//
// Created by Vinnie on 9/04/2025.
//

module;

#include "slang/include/slang-com-ptr.h"
#include "slang/include/slang.h"

export module Engine.ShaderCompiler;
import std;
import Engine.WGPU;
import Engine.Assert;
import Engine.ShaderCompiler.Types;

// Compiles a slang shader to SPIRV
// Parses slang reflection data
export class ShaderCompiler {
public:
     enum SlangTargetCompileFlag {
          WGSL,
          SPIRV
     };

     // Compile a shader from its module name (NOT PATH, just its filename)
     explicit ShaderCompiler(const std::string& shader_name, bool bForceRecompile = false, SlangTargetCompileFlag target = SPIRV);
     wgpu::raii::ShaderModule GetCompiledShaderModule();
     BindingLayouts GetPipelineLayout();
     std::vector<UniformMetadata> GetUniformMetadata();

     struct CompiledShaderModule{
          std::string name{};
          wgpu::raii::ShaderModule compiledShaderModule{};
          std::vector<UniformMetadata> compiledMetadata{};
          BindingLayouts compiledLayout{};
     };
private:
     static WGPUTextureSampleType MapSlangToTextureSampleFormat(slang::TypeReflection::ScalarType fmt);
     static WGPUTextureFormat MapSlangToTextureFormat(slang::TypeReflection::ScalarType fmt, int elemCount);

     CompiledShaderModule CompiledShader;

     BindingLayouts ComposeBindingData(Slang::ComPtr<slang::IComponentType> program);
     std::vector<const char*> GetShaderDirectories();

     // Parse a top-level globally defined shader parameter. May be a ParameterBlock which expands out to more vars
     std::vector<UniformMetadata> ParseShaderParameter(slang::VariableLayoutReflection* var);
     // Parse a variable which could contain a resource or parameterblock
     std::vector<UniformMetadata> ParseShaderVar(slang::TypeLayoutReflection *typeLayout, int group, int slot);
     // Parse out a single resource (texture, constantBuffer, structuredbuffer)
     UniformMetadata ParseResource(slang::TypeLayoutReflection *typeLayout, int group, int slot);
     // Expand a parameterblock into its sub-data
     std::vector<UniformMetadata> ParseParameterBlock(slang::TypeLayoutReflection *paramBlock, int group, int slot);
     // Parse out struct subfields for a constBuffer
     std::vector<UniformMetadata::BufferField> ParseBufferSubfields(slang::TypeLayoutReflection* typeLayout);

     Slang::ComPtr<slang::ISession> session;

     // Shader search directories
     static inline std::vector<const char*> shaderSources = {"/engine/", "/res/"};

     // Spirv output
     Slang::ComPtr<slang::IBlob> spirv;
     // WGSL output
     std::string wgsl;

     // Global shader cache
     static std::unordered_map<std::string, CompiledShaderModule>& GetShaderCache();;
};


// Helper for building pipelines and bindings
class ShaderObjectLayoutBuilder {
public:
     void AddBindingsFrom(UniformMetadata* entry, slang::TypeLayoutReflection* typeLayout);
     std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> CreateBindGroupLayouts();
     BindingLayouts CreatePipelineLayout();

     std::unordered_map<uint32_t, std::vector<wgpu::BindGroupLayoutEntry>> entries;
     uint32_t m_bindingIndex = 0;

     struct PushConstantData {
          uint32_t Size = 0;
          WGPUShaderStage Visibility = wgpu::ShaderStage::None;
     };

     std::optional<PushConstantData> pushConstant{};
};


