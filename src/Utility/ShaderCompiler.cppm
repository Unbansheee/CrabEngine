//
// Created by Vinnie on 9/04/2025.
//

module;

#include "slang/include/slang-com-ptr.h"
#include "slang/include/slang.h"


export module Engine.ShaderCompiler;
import std;
import Engine.WGPU;
import Engine.Application;
import Engine.Assert;

export struct ShaderBindings {
     struct Entry {
          std::string Name = "Binding";
          uint8_t Group = 0;
          uint8_t Location = 0;
          uint32_t SizeInBytes = 0;
          WGPUShaderStage Visibility;
          bool IsPushConstant = false;
     };
};

export class ShaderCompiler {
public:
     enum SlangTargetCompileFlag {
          WGSL,
          SPIRV
     };

     ShaderCompiler(const std::string& shader_name, SlangTargetCompileFlag target = SPIRV);
     wgpu::raii::ShaderModule GetCompiledShaderModule();
     wgpu::raii::PipelineLayout GetPipelineLayout();
private:
     wgpu::raii::PipelineLayout ComposeBindingData(Slang::ComPtr<slang::IComponentType> program);
     std::vector<const char*> GetShaderDirectories();
     ShaderBindings::Entry ParseShaderParameter(slang::VariableLayoutReflection* var);

     Slang::ComPtr<slang::ISession> session;
     static inline std::vector<const char*> shaderSources = {ENGINE_RESOURCE_DIR};

     wgpu::raii::ShaderModule compiledShaderModule;
     wgpu::raii::PipelineLayout compiledLayout;

     Slang::ComPtr<slang::IBlob> spirv;
     std::string wgsl;
};

class ShaderObjectLayoutBuilder {
public:
     void AddBindingsFrom(ShaderBindings::Entry* entry, slang::TypeLayoutReflection* typeLayout, uint32_t descriptorCount);
     std::vector<wgpu::raii::BindGroupLayout> CreateBindGroupLayouts();
     wgpu::raii::PipelineLayout CreatePipelineLayout();

     std::unordered_map<uint32_t, std::vector<wgpu::BindGroupLayoutEntry>> entries;
     uint32_t m_bindingIndex = 0;

     struct PushConstantData {
          uint32_t Size;
          WGPUShaderStage Visibility;
     };

     std::optional<PushConstantData> pushConstant;
};


wgpu::raii::PipelineLayout ShaderObjectLayoutBuilder::CreatePipelineLayout() {
     auto device = Application::Get().GetDevice();

     wgpu::PipelineLayoutDescriptor desc;
     auto bgLayouts = CreateBindGroupLayouts();
     desc.bindGroupLayouts = (WGPUBindGroupLayout*)bgLayouts.data();
     desc.bindGroupLayoutCount = bgLayouts.size();

     if (pushConstant.has_value()) {
          auto pc = pushConstant.value();
          wgpu::PushConstantRange pcRange;
          pcRange.start = 0;
          pcRange.end = pc.Size;
          pcRange.stages = pc.Visibility;

          wgpu::PipelineLayoutExtras extras = wgpu::Default;
          extras.pushConstantRanges = &pcRange;
          extras.pushConstantRangeCount = 1;

          desc.nextInChain = &extras.chain;
          return device.createPipelineLayout(desc);
     }

     return device.createPipelineLayout(desc);
}

std::vector<wgpu::raii::BindGroupLayout> ShaderObjectLayoutBuilder::CreateBindGroupLayouts() {
     auto device = Application::Get().GetDevice();
     std::vector<wgpu::raii::BindGroupLayout> layouts;
     for (auto e : entries) {
          wgpu::BindGroupLayoutDescriptor desc;
          desc.entries = (WGPUBindGroupLayoutEntry*)e.second.data();
          desc.entryCount = e.second.size();
          layouts.emplace_back(device.createBindGroupLayout(desc));
     }
     return layouts;
}


WGPUTextureSampleType MapSlangToTextureSampleFormat(slang::TypeReflection::ScalarType fmt) {
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
WGPUTextureFormat MapSlangToTextureFormat(slang::TypeReflection::ScalarType fmt, int elemCount) {
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




void ShaderObjectLayoutBuilder::AddBindingsFrom(ShaderBindings::Entry* entry, slang::TypeLayoutReflection *typeLayout, uint32_t descriptorCount) {
     int bindingRangeCount = typeLayout->getBindingRangeCount();
     uint32_t bindingGroup = 0;
     uint32_t bindingSlot = 0;


     auto bindingType = typeLayout->getBindingRangeType(bindingGroup);
     bool bIsPushConstant = bindingType == slang::BindingType::PushConstant;
     if (bIsPushConstant) {
          auto elemType = typeLayout->getElementTypeLayout();
          auto elemSize = elemType->getSize();
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
     switch (kind) {
          case slang::TypeReflection::Kind::ConstantBuffer:
               WGPUBufferBindingLayout constBufferBinding;
               constBufferBinding.type = WGPUBufferBindingType_Uniform;
               constBufferBinding.minBindingSize = entry->SizeInBytes;
               constBufferBinding.hasDynamicOffset = false;
               e.buffer = constBufferBinding;
               break;
          case slang::TypeReflection::Kind::ParameterBlock:
               WGPUBufferBindingLayout paramBlockBinding;
               paramBlockBinding.type = WGPUBufferBindingType_Uniform;
               paramBlockBinding.minBindingSize = entry->SizeInBytes;
               paramBlockBinding.hasDynamicOffset = false;
               e.buffer = paramBlockBinding;
               break;
          case slang::TypeReflection::Kind::Resource: // Texture, StructuredBuffer
               if (typeLayout->getResourceShape() & SlangResourceShape::SLANG_STRUCTURED_BUFFER && typeLayout->getResourceShape() &! SlangResourceShape::SLANG_TEXTURE_2D) {
                    WGPUBufferBindingLayout storageBinding;
                    storageBinding.type = typeLayout->getResourceAccess() & SlangResourceAccess::SLANG_RESOURCE_ACCESS_WRITE ? WGPUBufferBindingType_Storage : WGPUBufferBindingType_ReadOnlyStorage;
                    storageBinding.minBindingSize = entry->SizeInBytes;
                    storageBinding.hasDynamicOffset = false;
                    e.buffer = storageBinding;
                    break;
               }
               else if (typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_2D) {
                    SlangResourceAccess access = typeLayout->getResourceAccess();
                    bool isStorageTexture =
                        (access & SLANG_RESOURCE_ACCESS_READ_WRITE) ||
                        (access & SLANG_RESOURCE_ACCESS_WRITE);

                    auto t = typeLayout->getResourceResultType();
                    auto elemCount = t->getElementCount();
                    auto elementType = t->getScalarType();

                    auto format = MapSlangToTextureFormat(elementType, elemCount);
                    auto sampleType = MapSlangToTextureSampleFormat(elementType);


                    if (isStorageTexture) {
                         WGPUStorageTextureBindingLayout storageTextureBinding;
                         auto fmt = typeLayout->getBindingRangeImageFormat(entry->Location);
                         storageTextureBinding.format = format;
                         storageTextureBinding.viewDimension = wgpu::TextureViewDimension::_2D;
                         storageTextureBinding.access = typeLayout->getResourceAccess() & SlangResourceAccess::SLANG_RESOURCE_ACCESS_WRITE ? WGPUStorageTextureAccess_ReadWrite : WGPUStorageTextureAccess_ReadOnly;
                         e.storageTexture = storageTextureBinding;
                         break;
                    }
                    else {
                         WGPUTextureBindingLayout textureBinding;
                         auto fmt = typeLayout->getBindingRangeImageFormat(typeLayout->getBindingRangeCount());
                         textureBinding.sampleType = sampleType;
                         textureBinding.multisampled = typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_2D_MULTISAMPLE || typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_MULTISAMPLE_FLAG ? WGPUOptionalBool_True : WGPUOptionalBool_False;
                         textureBinding.viewDimension = wgpu::TextureViewDimension::_2D;
                         e.texture = textureBinding;
                         break;
                    }
               }
          default:
               Assert::Fail("Not Implemented");
     }

     entries[entry->Group].push_back(e);
}
