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
        std::cout << "Shader compile successful" << std::endl;

        wgpu::ShaderSourceSPIRV shaderCodeDesc = wgpu::Default;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceSPIRV;
        shaderCodeDesc.code = static_cast<uint32_t const*>(spirv->getBufferPointer());
        shaderCodeDesc.codeSize = spirv->getBufferSize() / sizeof(uint32_t);
        shaderDesc.nextInChain = &shaderCodeDesc.chain;
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
        std::cout << "Shader compile successful" << std::endl;

        wgpu::ShaderSourceWGSL shaderCodeDesc = wgpu::Default;
        shaderCodeDesc.chain.next = nullptr;
        shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
        shaderCodeDesc.code = {wgsl.c_str(), wgsl.length()};
        shaderDesc.nextInChain = &shaderCodeDesc.chain;
        std::cout << wgsl << std::endl;
    }

    compiledShaderModule = device.createShaderModule(shaderDesc);
    Assert::Check(*compiledShaderModule != nullptr, "module != nullptr", "Error compiling module");

    compiledLayout = ComposeBindingData(composedProgram);

    for (auto dir : dirs) {
        delete[] dir;
    }
}

wgpu::raii::ShaderModule ShaderCompiler::GetCompiledShaderModule() {
    return compiledShaderModule;
}

wgpu::raii::PipelineLayout ShaderCompiler::GetPipelineLayout() {
    return compiledLayout;
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

wgpu::raii::PipelineLayout ShaderCompiler::ComposeBindingData(ComPtr<slang::IComponentType> program) {
    slang::ProgramLayout* programLayout = program->getLayout(0);
    ShaderObjectLayoutBuilder builder;

    for (int i = 0; i < programLayout->getParameterCount(); i++) {
        auto p = programLayout->getParameterByIndex(i);

        auto entry = ParseShaderParameter(p);
        slang::TypeLayoutReflection* ref = p->getTypeLayout();
        builder.AddBindingsFrom(&entry, ref, 0);

    }
    return builder.CreatePipelineLayout();;
}

ShaderBindings::Entry ShaderCompiler::ParseShaderParameter(slang::VariableLayoutReflection *p) {
    auto group = p->getBindingSpace();
    auto binding = p->getBindingIndex();
    auto type = p->getType();
    auto kind = type->getKind();
    auto shape = type->getResourceShape();
    auto access = type->getResourceAccess();
    auto category = p->getCategory();

    uint32_t bindGroup = 0;
    uint32_t bindSlot = 0;
    bindGroup = p->getOffset(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_REGISTER_SPACE);
    bindSlot = p->getOffset(SlangParameterCategory::SLANG_PARAMETER_CATEGORY_DESCRIPTOR_TABLE_SLOT);

    WGPUShaderStage stage = wgpu::ShaderStage::None;
    /*
    auto slangStage = p->getStage();
    if (slangStage & SlangStage::SLANG_STAGE_VERTEX) {
        stage |= wgpu::ShaderStage::Vertex;
    }
    if (slangStage & SlangStage::SLANG_STAGE_FRAGMENT) {
        stage |= wgpu::ShaderStage::Fragment;
    }
    if (slangStage & SlangStage::SLANG_STAGE_COMPUTE) {
        stage |= wgpu::ShaderStage::Compute;
    };
    */
    stage |= wgpu::ShaderStage::Vertex;
    stage |= wgpu::ShaderStage::Fragment;

    Assert::Check(stage != wgpu::ShaderStage::None, "stage != None", "No shader stages defined for " + std::string(p->getName()));

    ShaderBindings::Entry entry;
    entry.Name = p->getName();
    entry.SizeInBytes = p->getTypeLayout()->getSize();
    entry.Location = p->getBindingIndex();
    entry.Group = p->getBindingSpace(DescriptorTableSlot);
    entry.Visibility = stage;


    return entry;
}


