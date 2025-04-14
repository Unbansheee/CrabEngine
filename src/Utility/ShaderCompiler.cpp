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

using namespace Slang;
using namespace slang;

ShaderCompiler::ShaderCompiler(const std::string &shader_name, SlangTargetCompileFlag target) {

    static Slang::ComPtr<IGlobalSession> globalSession;
    if (!globalSession) {
        SlangGlobalSessionDesc desc = {};
        createGlobalSession(&desc, globalSession.writeRef());
    }

    if (shaderCache.contains(shader_name)) {
        CompiledShader = shaderCache.at(shader_name);
        return;
    }

    TargetDesc targetDesc;
    if (target == SlangTargetCompileFlag::WGSL) {
        targetDesc.format = SlangCompileTarget::SLANG_WGSL;
        targetDesc.profile = globalSession->findProfile("sm_6_5");
    }
    else if (target == SlangTargetCompileFlag::SPIRV) {
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
    sessionDesc.compilerOptionEntryCount = options.size();

    globalSession->createSession(sessionDesc, session.writeRef());
    Assert::Check(session != nullptr, "session != nullptr", "Error creating Shader Compiler session");

    slang::IModule* slangModule = nullptr;
    {
        ComPtr<IBlob> diag;
        slangModule = session->loadModule(shader_name.c_str(), diag.writeRef());
        if (diag) {
            std::cerr << std::string((const char*)diag->getBufferPointer()) << std::endl;
        }
    }

    Assert::Check(slangModule != nullptr, "slangModule != nullptr", "Error creating Slang shader module for " + shader_name);

    std::vector<slang::IComponentType*> componentTypes;
    componentTypes.push_back(slangModule);

    ComPtr<slang::IComponentType> composedProgram;
    {
        ComPtr<slang::IBlob> diagnosticBlob;
        SlangResult result = session->createCompositeComponentType(
                componentTypes.data(),
                componentTypes.size(),
                composedProgram.writeRef(),
                diagnosticBlob.writeRef());

        if (diagnosticBlob) {
            Assert::Fail(static_cast<const char*>(diagnosticBlob->getBufferPointer()));
        }
    }

    Assert::Check(composedProgram != nullptr, "composedProgram != nullptr", "Error composing Slang program " + shader_name);

    wgpu::ShaderModuleDescriptor shaderDesc = wgpu::Default;
    shaderDesc.label = {shader_name.c_str(), shader_name.length()};
    auto device = Application::Get().GetDevice();

    if (target == SlangTargetCompileFlag::SPIRV) {
        composedProgram->getTargetCode(0, spirv.writeRef());
        std::cout << "Slang -> SPIRV Shader compile successful" << std::endl;

        wgpu::ShaderSourceSPIRV shaderCodeDesc = wgpu::Default;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceSPIRV;
        shaderCodeDesc.code = static_cast<uint32_t const*>(spirv->getBufferPointer());
        shaderCodeDesc.codeSize = spirv->getBufferSize() / sizeof(uint32_t);;
        shaderDesc.nextInChain = (WGPUChainedStruct*)&shaderCodeDesc.chain;
        CompiledShader.compiledShaderModule = device.createShaderModule(shaderDesc);
    }


    else if (target == SlangTargetCompileFlag::WGSL) {
        ComPtr<IBlob> out;
        {
            ComPtr<IBlob> diag;
            composedProgram->getTargetCode(0, out.writeRef(), diag.writeRef());
            if (diag) {
                Assert::Fail((const char*)diag->getBufferPointer());
            }
        }

        wgsl = std::string((const char*)out->getBufferPointer());
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

    shaderCache.insert({shader_name, CompiledShader});

    for (auto dir : dirs) {
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

std::vector<const char*> ShaderCompiler::GetShaderDirectories() {
    std::vector<const char*> result;
    for (auto path : shaderSources) {
        char* str = new char[strlen(path) + 1];
        strcpy(str, path);
        result.push_back(str);

        std::filesystem::path p = path;
        auto it = std::filesystem::recursive_directory_iterator(p);
        for (auto dir : it) {
            if (dir.is_directory()) {
                char* str = new char[dir.path().string().size() + 1];
                strcpy(str, dir.path().string().c_str());
                result.push_back(str);
            }
        }
    }

    return result;
}

BindingLayouts ShaderCompiler::ComposeBindingData(ComPtr<slang::IComponentType> program) {
    slang::ProgramLayout* programLayout = program->getLayout(0);
    ShaderObjectLayoutBuilder builder;

    for (int i = 0; i < programLayout->getParameterCount(); i++) {
        auto p = programLayout->getParameterByIndex(i);

        auto entries = ParseShaderParameter(p);
        for (auto& entry : entries) {
            slang::TypeLayoutReflection* ref = p->getTypeLayout();
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

    auto entries = ParseShaderVar(typeLayout, group, location);
    auto isPC = p->getTypeLayout()->getBindingRangeType(0) == slang::BindingType::PushConstant;



    if (entries.size() == 1) {
        WGPUShaderStage stage = wgpu::ShaderStage::None;
        stage |= wgpu::ShaderStage::Vertex;
        stage |= wgpu::ShaderStage::Fragment;

        auto& entry = entries.front();
        entry.Name = p->getName();
        //entry.SizeInBytes = typeLayout->getSize();
        entry.Visibility = stage;
    }

    return entries;
}

std::vector<UniformMetadata> ShaderCompiler::ParseShaderVar(slang::TypeLayoutReflection *typeLayout, int group, int slot) {
    if (typeLayout->getKind() == TypeReflection::Kind::ParameterBlock) {
        return ParseParameterBlock(typeLayout, group, slot);
    }
    else return { ParseResource(typeLayout, group, slot) };
}

UniformMetadata ShaderCompiler::ParseResource(slang::TypeLayoutReflection *typeLayout, int group, int slot) {
    UniformMetadata entry;

    auto kind = typeLayout->getKind();

    uint32_t bindingGroup = 0; // entry.Group
    auto bindingType = typeLayout->getBindingRangeType(bindingGroup);
    entry.IsPushConstant = bindingType == slang::BindingType::PushConstant;

    slang::TypeReflection::Kind k;
    slang::TypeLayoutReflection* l;

    switch (kind) {
        case slang::TypeReflection::Kind::ConstantBuffer:
            entry.BindingType = WGPUBindingType::Buffer;
            if (typeLayout->getElementTypeLayout()->getKind() == slang::TypeReflection::Kind::ParameterBlock) {
                return ParseParameterBlock(typeLayout->getElementTypeLayout(), group, slot).front();
            }

            entry.SizeInBytes = typeLayout->getElementTypeLayout()->getSize();
            break;

        case slang::TypeReflection::Kind::Resource: // Texture, StructuredBuffer
            if (typeLayout->getResourceShape() & SlangResourceShape::SLANG_STRUCTURED_BUFFER && typeLayout->getResourceShape() &! SlangResourceShape::SLANG_TEXTURE_2D) {
                entry.BindingType = WGPUBindingType::Buffer;
            }
            else if (typeLayout->getResourceShape() & SlangResourceShape::SLANG_TEXTURE_2D) {
                SlangResourceAccess access = typeLayout->getResourceAccess();

                auto t = typeLayout->getResourceResultType();
                auto elemCount = t->getElementCount();
                auto elementType = t->getScalarType();

                bool isStorageTexture =
                    (access & SLANG_RESOURCE_ACCESS_READ_WRITE) ||
                    (access & SLANG_RESOURCE_ACCESS_WRITE);
                entry.StorageTextureAccess = isStorageTexture ? wgpu::StorageTextureAccess::ReadWrite : wgpu::StorageTextureAccess::Undefined;
                entry.BindingType = isStorageTexture ? WGPUBindingType::StorageTexture : WGPUBindingType::Texture;
                entry.Format = ShaderCompiler::MapSlangToTextureFormat(elementType, (int)elemCount);
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

std::vector<UniformMetadata> ShaderCompiler::ParseParameterBlock(slang::TypeLayoutReflection *paramBlock, int group, int slot) {
    slang::TypeLayoutReflection* typeLayout = paramBlock;
    slang::TypeLayoutReflection* elementTypeLayout = typeLayout->getElementTypeLayout();

    std::vector<UniformMetadata> subEntries;

    // TODO: Construct buffer uniform
    if (elementTypeLayout->getSize() > 0) {
        UniformMetadata subBuf;
        subBuf.Name = paramBlock->getName();
        subBuf.SizeInBytes = elementTypeLayout->getSize();
        subBuf.BindingType = WGPUBindingType::Buffer;
        subBuf.Visibility = wgpu::ShaderStage::Fragment | wgpu::ShaderStage::Vertex;
        subBuf.Group = group;
        subBuf.Location = slot;
        subEntries.push_back(subBuf);
    }

    int subObjectRangeCount = elementTypeLayout->getBindingRangeCount();
    for (int subObjectRangeIndex = 0; subObjectRangeIndex < subObjectRangeCount;
         ++subObjectRangeIndex)
    {
        auto bindingRangeIndex = subObjectRangeIndex;
           //elementTypeLayout->getSubObjectRangeBindingRangeIndex(subObjectRangeIndex);
        auto bindingType = elementTypeLayout->getBindingRangeType(bindingRangeIndex);
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


BindingLayouts ShaderObjectLayoutBuilder::CreatePipelineLayout() {
    auto device = Application::Get().GetDevice();

    BindingLayouts out;

    wgpu::PipelineLayoutDescriptor desc;
    out.BindGroupLayouts = CreateBindGroupLayouts();
    std::vector<wgpu::raii::BindGroupLayout> bgLayouts;
    for (auto& l : out.BindGroupLayouts) {
        bgLayouts.push_back(l.second);
    }

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
        out.PipelineLayout = device.createPipelineLayout(desc);
        return out;
    }

    out.PipelineLayout = device.createPipelineLayout(desc);
    return out;
}

std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> ShaderObjectLayoutBuilder::CreateBindGroupLayouts() {
    auto device = Application::Get().GetDevice();
    std::unordered_map<uint32_t, wgpu::raii::BindGroupLayout> layouts;
    for (auto e : entries) {
        wgpu::BindGroupLayoutDescriptor desc;
        desc.entries = (WGPUBindGroupLayoutEntry*)e.second.data();
        desc.entryCount = e.second.size();
        layouts.insert({(uint32_t)e.first, device.createBindGroupLayout(desc)});
    }
    return layouts;
}