//
// Created by Vinnie on 9/04/2025.
//
module;
#include "include/slang.h"
#include "include/slang-com-ptr.h"
#include "include/slang-cpp-types.h"
#include "include/slang-gfx.h"

module Engine.ShaderCompiler;
import Engine.Assert;
import Engine.WGPU;
import Engine.Application;
import Engine.Filesystem;

using namespace Slang;
using namespace slang;

constexpr UniformMetadata::BufferField::FieldType MapScalarTypeToBufferReflectionType(
    slang::TypeReflection::ScalarType t) {
    switch (t) {
        case TypeReflection::None:
            Assert::Fail("Invalid ScalarType");
        case TypeReflection::Void:
            Assert::Fail("Invalid ScalarType");
        case TypeReflection::Bool:
            return UniformMetadata::BufferField::FieldType::BOOL;
        case TypeReflection::Int32:
            return UniformMetadata::BufferField::FieldType::INT;
        case TypeReflection::UInt32:
            return UniformMetadata::BufferField::FieldType::UINT;
        case TypeReflection::Int64:
            return UniformMetadata::BufferField::FieldType::INT64;
        case TypeReflection::UInt64:
            return UniformMetadata::BufferField::FieldType::UINT64;
        case TypeReflection::Float16:
            return UniformMetadata::BufferField::FieldType::FLOAT16;
        case TypeReflection::Float32:
            return UniformMetadata::BufferField::FieldType::FLOAT;
        case TypeReflection::Float64:
            return UniformMetadata::BufferField::FieldType::FLOAT64;
        case TypeReflection::Int8:
            return UniformMetadata::BufferField::FieldType::INT8;
        case TypeReflection::UInt8:
            return UniformMetadata::BufferField::FieldType::UINT8;
        case TypeReflection::Int16:
            return UniformMetadata::BufferField::FieldType::INT16;
        case TypeReflection::UInt16:
            return UniformMetadata::BufferField::FieldType::UINT16;
    }

    return UniformMetadata::BufferField::UNDEFINED;
}

ShaderCompiler::ShaderCompiler(const std::string &shader_name, bool bForceRecompile, SlangTargetCompileFlag target) {
    static Slang::ComPtr<IGlobalSession> globalSession;
    if (!globalSession) {
        SlangGlobalSessionDesc desc = {};
        createGlobalSession(&desc, globalSession.writeRef());
    }

    if (!bForceRecompile && GetShaderCache().contains(shader_name)) {
        CompiledShader = GetShaderCache().at(shader_name);
        return;
    }

    TargetDesc targetDesc;
    if (target == SlangTargetCompileFlag::WGSL) {
        targetDesc.format = SlangCompileTarget::SLANG_WGSL;
        targetDesc.profile = globalSession->findProfile("sm_6_5");
    } else if (target == SlangTargetCompileFlag::SPIRV) {
        targetDesc.format = SlangCompileTarget::SLANG_SPIRV;
        targetDesc.profile = globalSession->findProfile("sm_6_5");
    }

    SessionDesc sessionDesc;
    auto dirs = GetShaderDirectories();
    sessionDesc.searchPaths = dirs.data();
    sessionDesc.searchPathCount = dirs.size();
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.flags = 0;

    std::vector<slang::CompilerOptionEntry> options;

    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = static_cast<uint32_t>(options.size());

    globalSession->createSession(sessionDesc, session.writeRef());
    Assert::Check(session != nullptr, "session != nullptr", "Error creating Shader Compiler session");

    slang::IModule *slangModule = nullptr; {
        ComPtr<IBlob> diag;
        slangModule = session->loadModule(shader_name.c_str(), diag.writeRef());
        if (diag) {
            std::cerr << std::string((const char *) diag->getBufferPointer()) << std::endl;
        }
    }

    Assert::Check(slangModule != nullptr, "slangModule != nullptr",
                  "Error creating Slang shader module for " + shader_name);

    std::vector<slang::IComponentType *> componentTypes;
    componentTypes.push_back(slangModule);

    ComPtr<slang::IComponentType> composedProgram; {
        ComPtr<slang::IBlob> diagnosticBlob;
        session->createCompositeComponentType(
            componentTypes.data(),
            componentTypes.size(),
            composedProgram.writeRef(),
            diagnosticBlob.writeRef());

        if (diagnosticBlob) {
            Assert::Fail(static_cast<const char *>(diagnosticBlob->getBufferPointer()));
        }
    }

    Assert::Check(composedProgram != nullptr, "composedProgram != nullptr",
                  "Error composing Slang program " + shader_name);

    wgpu::ShaderModuleDescriptor shaderDesc = wgpu::Default;
    shaderDesc.label = {shader_name.c_str(), shader_name.length()};
    auto device = Application::Get().GetDevice();

    if (target == SlangTargetCompileFlag::SPIRV) {
        composedProgram->getTargetCode(0, spirv.writeRef());
        std::cout << "Slang -> SPIRV Shader compile successful" << std::endl;

        wgpu::ShaderSourceSPIRV shaderCodeDesc = wgpu::Default;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceSPIRV;
        shaderCodeDesc.code = static_cast<uint32_t const *>(spirv->getBufferPointer());
        shaderCodeDesc.codeSize = static_cast<uint32_t>(spirv->getBufferSize()) / sizeof(uint32_t);;
        shaderDesc.nextInChain = (WGPUChainedStruct *) &shaderCodeDesc.chain;
        CompiledShader.compiledShaderModule = device.createShaderModule(shaderDesc);
    } else if (target == SlangTargetCompileFlag::WGSL) {
        ComPtr<IBlob> out; {
            ComPtr<IBlob> diag;
            composedProgram->getTargetCode(0, out.writeRef(), diag.writeRef());
            if (diag) {
                Assert::Fail((const char *) diag->getBufferPointer());
            }
        }

        wgsl = std::string((const char *) out->getBufferPointer());
        std::cout << "Slang -> WGSL Shader compile successful" << std::endl;

        wgpu::ShaderSourceWGSL shaderCodeDesc = wgpu::Default;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
        shaderCodeDesc.code = {wgsl.c_str(), wgsl.length()};
        shaderDesc.nextInChain = &shaderCodeDesc.chain;
        CompiledShader.compiledShaderModule = device.createShaderModule(shaderDesc);
    }


    Assert::Check(*CompiledShader.compiledShaderModule != nullptr, "module != nullptr", "Error compiling module");

    CompiledShader.compiledLayout = ComposeBindingData(composedProgram);

    GetShaderCache().insert({shader_name, CompiledShader});

    for (auto dir: dirs) {
        delete[] dir;
    }
}

wgpu::raii::ShaderModule ShaderCompiler::GetCompiledShaderModule() {
    return CompiledShader.compiledShaderModule;
}

BindingLayouts ShaderCompiler::GetPipelineLayout() {
    return CompiledShader.compiledLayout;
}

std::vector<UniformMetadata> ShaderCompiler::GetUniformMetadata() {
    return CompiledShader.compiledMetadata;
}

// TODO: Cleanup strings
std::vector<const char *> ShaderCompiler::GetShaderDirectories() {
    std::vector<const char *> result;
    for (auto path: shaderSources) {
        auto absPath = Filesystem::AbsolutePath(path);
        char* str = new char[absPath.size() + 1];
        strcpy_s(str, absPath.size() + 1, absPath.c_str());
        result.push_back(str);

        std::filesystem::path p = absPath;
        auto it = std::filesystem::recursive_directory_iterator(p);
        for (auto dir: it) {
            if (dir.is_directory()) {
                std::string pathStr = dir.path().string();
                char* pathstr = new char[pathStr.size() + 1];
                strcpy_s(pathstr, pathStr.size() + 1, pathStr.c_str());
                result.push_back(pathstr);
            }
        }
    }

    return result;
}

BindingLayouts ShaderCompiler::ComposeBindingData(ComPtr<slang::IComponentType> program) {
    slang::ProgramLayout *programLayout = program->getLayout(0);
    ShaderObjectLayoutBuilder builder;

    for (uint32_t i = 0; i < programLayout->getParameterCount(); i++) {
        auto p = programLayout->getParameterByIndex(i);

        auto entries = ParseShaderParameter(p);
        for (auto &entry: entries) {
            slang::TypeLayoutReflection *ref = p->getTypeLayout();
            builder.AddBindingsFrom(&entry, ref);
            CompiledShader.compiledMetadata.push_back(entry);
        }
    }

    return builder.CreatePipelineLayout();
}

std::vector<UniformMetadata> ShaderCompiler::ParseShaderParameter(slang::VariableLayoutReflection *p) {
    auto typeLayout = p->getTypeLayout();

    auto group = p->getOffset(ParameterCategory::SubElementRegisterSpace);
    auto location = p->getType()->getKind() == slang::TypeReflection::Kind::ParameterBlock ? 0 : p->getBindingIndex();

    auto entries = ParseShaderVar(typeLayout, static_cast<int>(group), location);
    auto isSerialized = false;
    for (uint32_t i = 0; i < p->getVariable()->getUserAttributeCount(); i++ ) {
        auto attribute = p->getVariable()->getUserAttributeByIndex(i);
        std::string name = attribute->getName();
        if (name == "SerializedField") {
            isSerialized = true;
            break;
        }
    }

    for (auto& entry : entries) {
        entry.IsSerialized = isSerialized;
    }

    if (entries.size() == 1) {
        WGPUShaderStage stage = wgpu::ShaderStage::None;
        stage |= wgpu::ShaderStage::Vertex;
        stage |= wgpu::ShaderStage::Fragment;

        auto &entry = entries.front();
        entry.Name = p->getName();
        entry.Visibility = stage;
    }

    return entries;
}

std::vector<UniformMetadata> ShaderCompiler::ParseShaderVar(slang::TypeLayoutReflection *typeLayout, int group,
                                                            int slot) {
    if (typeLayout->getKind() == TypeReflection::Kind::ParameterBlock) {
        return ParseParameterBlock(typeLayout, group, slot);
    } else return {ParseResource(typeLayout, group, slot)};
}

UniformMetadata ShaderCompiler::ParseResource(slang::TypeLayoutReflection *typeLayout, int group, int slot) {
    UniformMetadata entry;

    auto kind = typeLayout->getKind();

    uint32_t bindingGroup = 0; // entry.Group
    auto bindingType = typeLayout->getBindingRangeType(bindingGroup);
    entry.IsPushConstant = bindingType == slang::BindingType::PushConstant;

    switch (kind) {
        case slang::TypeReflection::Kind::ConstantBuffer: {
            entry.BindingType = WGPUBindingType::Buffer;
            if (typeLayout->getElementTypeLayout()->getKind() == slang::TypeReflection::Kind::ParameterBlock) {
                return ParseParameterBlock(typeLayout->getElementTypeLayout(), group, slot).front();
            }
            entry.SizeInBytes = static_cast<uint32_t>(typeLayout->getElementTypeLayout()->getSize());
            entry.BufferFields = ParseBufferSubfields(typeLayout);
        }
        break;

        case slang::TypeReflection::Kind::Resource: // Texture, StructuredBuffer
            if (typeLayout->getResourceShape() & SlangResourceShape::SLANG_STRUCTURED_BUFFER && typeLayout->
                getResourceShape() & !SlangResourceShape::SLANG_TEXTURE_2D) {
                entry.BindingType = WGPUBindingType::Buffer;
            } else if (typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_2D) {
                SlangResourceAccess access = typeLayout->getResourceAccess();

                auto t = typeLayout->getResourceResultType();
                auto elemCount = t->getElementCount();
                auto elementType = t->getScalarType();

                bool isStorageTexture =
                        (access & SLANG_RESOURCE_ACCESS_READ_WRITE) ||
                        (access & SLANG_RESOURCE_ACCESS_WRITE);
                entry.StorageTextureAccess = isStorageTexture
                                                 ? wgpu::StorageTextureAccess::ReadWrite
                                                 : wgpu::StorageTextureAccess::Undefined;
                entry.BindingType = isStorageTexture ? WGPUBindingType::StorageTexture : WGPUBindingType::Texture;
                entry.Format = ShaderCompiler::MapSlangToTextureFormat(elementType, (int) elemCount);
                entry.SampleType = ShaderCompiler::MapSlangToTextureSampleFormat(elementType);
                entry.Dimension = WGPUTextureViewDimension_2D;
            }
            break;
        case slang::TypeReflection::Kind::SamplerState: // Texture, StructuredBuffer
            entry.BindingType = Sampler;
            entry.SamplerBindingType = WGPUSamplerBindingType_Filtering;
            break;
        default:
            Assert::Fail("Invalid Resource Type");
    }

    entry.Group = group;
    entry.Location = slot;
    WGPUShaderStage stage = wgpu::ShaderStage::None;
    stage |= wgpu::ShaderStage::Vertex;
    stage |= wgpu::ShaderStage::Fragment;
    entry.Visibility = stage;

    return entry;
}

std::vector<UniformMetadata> ShaderCompiler::ParseParameterBlock(slang::TypeLayoutReflection *paramBlock, int group,
                                                                 int slot) {
    slang::TypeLayoutReflection *typeLayout = paramBlock;
    slang::TypeLayoutReflection *elementTypeLayout = typeLayout->getElementTypeLayout();

    std::vector<UniformMetadata> subEntries;

    // TODO: Construct buffer uniform
    if (elementTypeLayout->getSize() > 0) {
        UniformMetadata subBuf;
        subBuf.Name = paramBlock->getName();
        subBuf.SizeInBytes = static_cast<uint32_t>(elementTypeLayout->getSize());
        subBuf.BindingType = WGPUBindingType::Buffer;
        subBuf.Visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
        subBuf.Group = group;
        subBuf.Location = slot;
        subBuf.BufferFields = ParseBufferSubfields(typeLayout);
        subEntries.push_back(subBuf);
    }

    int subObjectRangeCount = elementTypeLayout->getBindingRangeCount();
    for (int subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount;
         ++subObjectRangeIndex) {
        auto bindingRangeIndex = subObjectRangeIndex;
        auto leaf = elementTypeLayout->getBindingRangeLeafTypeLayout(bindingRangeIndex);
        auto nextVar = elementTypeLayout->getBindingRangeLeafVariable(bindingRangeIndex);

        auto v = ParseShaderVar(leaf, group, slot + subObjectRangeIndex);
        if (v.size() == 1) {
            v.front().Name = nextVar->getName();
        }
        subEntries.insert(subEntries.end(), v.begin(), v.end());
    }

    return subEntries;
}

std::vector<UniformMetadata::BufferField>
ShaderCompiler::ParseBufferSubfields(slang::TypeLayoutReflection *typeLayout) {
    std::vector<UniformMetadata::BufferField> fields;
    // Parse buffer fields
    uint32_t bufferFields;
    if (typeLayout->getKind() == TypeReflection::Kind::Struct) {
        bufferFields = typeLayout->getFieldCount();
    } else {
        bufferFields = typeLayout->getElementTypeLayout()->getFieldCount();
    }

    for (uint32_t i = 0; i < bufferFields; i++) {
        UniformMetadata::BufferField &Subfield = fields.emplace_back();
        VariableLayoutReflection *field = nullptr;
        if (typeLayout->getKind() == TypeReflection::Kind::Struct) {
            field = typeLayout->getFieldByIndex(i);
        } else {
            field = typeLayout->getElementTypeLayout()->getFieldByIndex(i);
        }
        Subfield.Offset = static_cast<uint32_t>(field->getOffset());
        Subfield.Name = field->getName();
        Subfield.SizeInBytes = static_cast<uint32_t>(field->getTypeLayout()->getSize());

        auto kind = field->getType()->getKind();

        if (kind == TypeReflection::Kind::Scalar) {
            auto scalarType = field->getType()->getScalarType();
            Subfield.Type = MapScalarTypeToBufferReflectionType(scalarType);
        }

        if (kind == TypeReflection::Kind::Struct) {
            Subfield.Type = UniformMetadata::BufferField::STRUCT;
            Subfield.Subfields = ParseBufferSubfields(field->getTypeLayout());
        }

        if (kind == TypeReflection::Kind::Vector) {
            auto vecElemCount = field->getType()->getElementCount();
            auto vecElemType = MapScalarTypeToBufferReflectionType(field->getType()->getElementType()->getScalarType());
            if (vecElemType == UniformMetadata::BufferField::FLOAT) {
                if (vecElemCount == 2) {
                    Subfield.Type = UniformMetadata::BufferField::VECTOR2;
                } else if (vecElemCount == 3) {
                    Subfield.Type = UniformMetadata::BufferField::VECTOR3;
                } else if (vecElemCount == 4) {
                    Subfield.Type = UniformMetadata::BufferField::VECTOR4;
                } else {
                    Assert::Fail("Unimplemented vector type");
                }
            } else if (vecElemType == UniformMetadata::BufferField::INT) {
                if (vecElemCount == 2) {
                    Subfield.Type = UniformMetadata::BufferField::VECTOR2I;
                } else if (vecElemCount == 3) {
                    Subfield.Type = UniformMetadata::BufferField::VECTOR3I;
                } else if (vecElemCount == 4) {
                    Subfield.Type = UniformMetadata::BufferField::VECTOR4I;
                } else {
                    Assert::Fail("Unimplemented vector type");
                }
            } else
                Assert::Fail("Unimplemented vector type");
        }
    }
    return fields;
}


BindingLayouts ShaderObjectLayoutBuilder::CreatePipelineLayout() {
    auto device = Application::Get().GetDevice();

    BindingLayouts out;

    wgpu::PipelineLayoutDescriptor desc;
    out.BindGroupLayouts = CreateBindGroupLayouts();
    std::vector<wgpu::raii::BindGroupLayout> bgLayouts;
    for (auto &l: out.BindGroupLayouts) {
        bgLayouts.push_back(l.second);
    }

    desc.bindGroupLayouts = (WGPUBindGroupLayout *) bgLayouts.data();
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
        out.PipelineLayout = device.createPipelineLayout(desc);
        return out;
    }

    out.PipelineLayout = device.createPipelineLayout(desc);
    return out;
}

std::unordered_map<std::string, ShaderCompiler::CompiledShaderModule> & ShaderCompiler::GetShaderCache() {
    static std::unordered_map<std::string, CompiledShaderModule> s_cache;
    return s_cache;
}

std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> ShaderObjectLayoutBuilder::CreateBindGroupLayouts() {
    auto device = Application::Get().GetDevice();
    std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> layouts;
    for (auto e: entries) {
        wgpu::BindGroupLayoutDescriptor desc;
        desc.entries = (WGPUBindGroupLayoutEntry *) e.second.data();
        desc.entryCount = e.second.size();
        layouts.insert({(uint32_t) e.first, device.createBindGroupLayout(desc)});
    }
    return layouts;
}


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

    return WGPUTextureSampleType_Undefined;
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

     if (entry->IsPushConstant) {
          auto elemSize = entry->SizeInBytes;
          pushConstant = PushConstantData{
               (uint32_t)elemSize,
               entry->Visibility
          };
     }

     wgpu::BindGroupLayoutEntry e;
     e.binding = entry->Location;
     e.visibility = entry->Visibility;
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
