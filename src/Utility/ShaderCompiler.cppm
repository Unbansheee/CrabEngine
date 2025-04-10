//
// Created by Vinnie on 9/04/2025.
//

module;

#include "slang/include/slang-com-ptr.h"
#include "slang/include/slang.h"


export module Engine.ShaderCompiler;
import std;
import Engine.WGPU;

export struct ShaderBindings {
     struct BindingLayoutInfo {
          uint32_t binding;
          WGPUBufferBindingType bufferType;  // Expand for textures/samplers
          WGPUShaderStage visibility;
     };

     using BindGroupLayout = std::vector<BindingLayoutInfo>;
     using PipelineLayout = std::vector<BindGroupLayout>;

     struct Entry {
          std::string Name = "Binding";
          uint8_t Group = 0;
          uint8_t Location = 0;
          uint32_t SizeInBytes = 0;
          std::optional<WGPUBufferBindingType> BufferBindingType{};
          std::optional<WGPUSamplerBindingType> SamplerBindingType{};
     };

     std::vector<Entry> Entries;
     wgpu::BindGroupLayout CreateBindGroupLayout();
};

export class ShaderCompiler {
public:
     enum SlangTargetCompileFlag {
          WGSL,
          SPIRV
     };

     ShaderCompiler(const std::string& shader_name, SlangTargetCompileFlag target = WGSL);
     wgpu::raii::ShaderModule GetCompiledShaderModule();
private:
     std::vector<const char*> GetShaderDirectories();

     ShaderBindings ComposeBindingData(Slang::ComPtr<slang::IComponentType> program);

     WGPUBufferBindingType MapSlangKindToWebGPUBuffer(SlangResourceShape kind, SlangResourceAccess);
     WGPUSamplerBindingType MapSlangKindToWebGPUSampler(SlangResourceShape kind);

     ShaderBindings::Entry ParseShaderParameter(slang::VariableLayoutReflection* var);

     Slang::ComPtr<slang::ISession> session;
     static inline std::vector<const char*> shaderSources = {ENGINE_RESOURCE_DIR};

     ShaderBindings bindings;

     wgpu::raii::ShaderModule compiledShaderModule;
     Slang::ComPtr<slang::IBlob> spirv;
     std::string wgsl;
};

