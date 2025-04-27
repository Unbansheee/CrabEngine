//
// Created by Vinnie on 9/04/2025.
//

module;

#include "slang/include/slang-com-ptr.h"
#include "slang/include/slang.h"


export module Engine.ShaderCompiler;
import std;
import Engine.WGPU;
//import Engine.Application;
import Engine.Assert;
import Engine.ShaderCompiler.Types;

export class ShaderCompiler {
public:
     enum SlangTargetCompileFlag {
          WGSL,
          SPIRV
     };

     ShaderCompiler(const std::string& shader_name, bool bForceRecompile = false, SlangTargetCompileFlag target = SPIRV);
     wgpu::raii::ShaderModule GetCompiledShaderModule();
     BindingLayouts GetPipelineLayout();
     std::vector<UniformMetadata> GetUniformMetadata();

     static WGPUTextureSampleType MapSlangToTextureSampleFormat(slang::TypeReflection::ScalarType fmt);
     static WGPUTextureFormat MapSlangToTextureFormat(slang::TypeReflection::ScalarType fmt, int elemCount);

private:
     struct CompiledShaderModule{
          std::string name{};
          wgpu::raii::ShaderModule compiledShaderModule{};
          std::vector<UniformMetadata> compiledMetadata{};
          BindingLayouts compiledLayout{};
     };

     CompiledShaderModule CompiledShader;

     BindingLayouts ComposeBindingData(Slang::ComPtr<slang::IComponentType> program);
     std::vector<const char*> GetShaderDirectories();
     std::vector<UniformMetadata> ParseShaderParameter(slang::VariableLayoutReflection* var);

     std::vector<UniformMetadata> ParseShaderVar(slang::TypeLayoutReflection *typeLayout, int group, int slot);
     UniformMetadata ParseResource(slang::TypeLayoutReflection *typeLayout, int group, int slot);
     std::vector<UniformMetadata> ParseParameterBlock(slang::TypeLayoutReflection *paramBlock, int group, int slot);
     std::vector<UniformMetadata::BufferField> ParseBufferSubfields(slang::TypeLayoutReflection* typeLayout);

     Slang::ComPtr<slang::ISession> session;
     static inline std::vector<const char*> shaderSources = {ENGINE_RESOURCE_DIR};

     Slang::ComPtr<slang::IBlob> spirv;
     std::string wgsl;


     static std::unordered_map<std::string, CompiledShaderModule>& GetShaderCache() {
          static std::unordered_map<std::string, CompiledShaderModule> s_cache;
          return s_cache;
     };
};



class ShaderObjectLayoutBuilder {
public:
     void AddBindingsFrom(UniformMetadata* entry, slang::TypeLayoutReflection* typeLayout);
     std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> CreateBindGroupLayouts();
     BindingLayouts CreatePipelineLayout();

     std::unordered_map<uint32_t, std::vector<wgpu::BindGroupLayoutEntry>> entries;
     uint32_t m_bindingIndex = 0;

     struct PushConstantData {
          uint32_t Size;
          WGPUShaderStage Visibility;
     };

     std::optional<PushConstantData> pushConstant;
};





WGPUTextureSampleType ShaderCompiler::MapSlangToTextureSampleFormat(slang::TypeReflection::ScalarType fmt) {
     switch (fmt) {
          case slang::TypeReflection::None:
          case slang::TypeReflection::Void:
               return WGPUTextureSampleType_Undefined;
          case slang::TypeReflection::Bool:
               return WGPUTextureSampleType_Uint;
          case slang::TypeReflection::Int32:
               return WGPUTextureSampleType_Sint;
          case slang::TypeReflection::UInt32:
               return WGPUTextureSampleType_Uint;
          case slang::TypeReflection::Int64:
               return WGPUTextureSampleType_Sint;
          case slang::TypeReflection::UInt64:
               return WGPUTextureSampleType_Uint;
          case slang::TypeReflection::Float16:
               return WGPUTextureSampleType_Float;
          case slang::TypeReflection::Float32:
               return WGPUTextureSampleType_Float;
          case slang::TypeReflection::Float64:
               return WGPUTextureSampleType_Float;
          case slang::TypeReflection::Int8:
               return WGPUTextureSampleType_Sint;
          case slang::TypeReflection::UInt8:
               return WGPUTextureSampleType_Uint;
          case slang::TypeReflection::Int16:
               return WGPUTextureSampleType_Sint;
          case slang::TypeReflection::UInt16:
               return WGPUTextureSampleType_Uint;
     }
}
WGPUTextureFormat ShaderCompiler::MapSlangToTextureFormat(slang::TypeReflection::ScalarType fmt, int elemCount) {
     switch (fmt) {
          case slang::TypeReflection::Int32:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R32Sint;
               if (elemCount == 2) return WGPUTextureFormat_RG32Sint;
               if (elemCount == 4) return WGPUTextureFormat_RGBA32Sint;
               break;

          case slang::TypeReflection::UInt32:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R32Uint;
               if (elemCount == 2) return WGPUTextureFormat_RG32Uint;
               if (elemCount == 4) return WGPUTextureFormat_RGBA32Uint;
               break;

          case slang::TypeReflection::Float16:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R16Float;
               if (elemCount == 2) return WGPUTextureFormat_RG16Float;
               if (elemCount == 4) return WGPUTextureFormat_RGBA16Float;
               break;

          case slang::TypeReflection::Float32:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R32Float;
               if (elemCount == 2) return WGPUTextureFormat_RG32Float;
               if (elemCount == 4) return WGPUTextureFormat_RGBA32Float;
               break;

          case slang::TypeReflection::Int8:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R8Sint;
               if (elemCount == 2) return WGPUTextureFormat_RG8Sint;
               if (elemCount == 4) return WGPUTextureFormat_RGBA8Sint;
               break;

          case slang::TypeReflection::UInt8:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R8Uint;
               if (elemCount == 2) return WGPUTextureFormat_RG8Uint;
               if (elemCount == 4) return WGPUTextureFormat_RGBA8Uint;
               break;

          case slang::TypeReflection::Int16:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R16Sint;
               if (elemCount == 2) return WGPUTextureFormat_RG16Sint;
               if (elemCount == 4) return WGPUTextureFormat_RGBA16Sint;
               break;

          case slang::TypeReflection::UInt16:
               if (elemCount == 1 || elemCount == 0) return WGPUTextureFormat_R16Uint;
               if (elemCount == 2) return WGPUTextureFormat_RG16Uint;
               if (elemCount == 4) return WGPUTextureFormat_RGBA16Uint;
               break;

          case slang::TypeReflection::Int64:
          case slang::TypeReflection::UInt64:
          case slang::TypeReflection::Float64:
          case slang::TypeReflection::Bool:
          case slang::TypeReflection::Void:
          case slang::TypeReflection::None:
          default:
               Assert::Fail("Invalid Texture Format for type: " + fmt);
              return WGPUTextureFormat_Undefined;
     }

     Assert::Fail("Invalid Texture Format for type: " + fmt);
     return WGPUTextureFormat_Undefined;
}


void ShaderObjectLayoutBuilder::AddBindingsFrom(UniformMetadata* entry, slang::TypeLayoutReflection *typeLayout) {
     int bindingRangeCount = typeLayout->getBindingRangeCount();

     if (entry->IsPushConstant) {
          auto elemType = typeLayout->getElementTypeLayout();
          auto elemSize = entry->SizeInBytes;
          pushConstant = PushConstantData{
               (uint32_t)elemSize,
               entry->Visibility
          };
     }

     wgpu::BindGroupLayoutEntry e;
     e.binding = entry->Location;
     e.visibility = entry->Visibility;
     auto n = typeLayout->getResourceResultType()->getName();
     auto kind = typeLayout->getKind();
     switch (entry->BindingType) {
          default:
               Assert::Fail("Not Implemented");
          case Buffer:
               WGPUBufferBindingLayout constBufferBinding;
               constBufferBinding.type = WGPUBufferBindingType_Uniform;
               constBufferBinding.minBindingSize = entry->SizeInBytes;
               constBufferBinding.hasDynamicOffset = false;
               e.buffer = constBufferBinding;
               break;
          case StorageTexture:
               WGPUStorageTextureBindingLayout storageTextureBinding;
               storageTextureBinding.format = entry->Format;
               storageTextureBinding.viewDimension = entry->Dimension;
               storageTextureBinding.access = entry->StorageTextureAccess;
               e.storageTexture = storageTextureBinding;
               break;

          case Texture:
               WGPUTextureBindingLayout textureBinding;
               //auto texfmt = typeLayout->getBindingRangeImageFormat(typeLayout->getBindingRangeCount());
               textureBinding.sampleType = entry->SampleType;
               textureBinding.multisampled = 0;//typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_2D_MULTISAMPLE || typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_MULTISAMPLE_FLAG ? WGPUOptionalBool_True : WGPUOptionalBool_False;
               textureBinding.viewDimension = entry->Dimension;
               e.texture = textureBinding;
               break;

          case Sampler:
               WGPUSamplerBindingLayout samplerBinding;
               samplerBinding.type = entry->SamplerBindingType;
               e.sampler = samplerBinding;
     }

     entries[entry->Group].push_back(e);
}
