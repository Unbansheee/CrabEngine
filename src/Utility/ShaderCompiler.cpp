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

ShaderCompiler::ShaderCompiler(const std::string &shader_name) {

    static Slang::ComPtr<IGlobalSession> globalSession;
    if (!globalSession) {
        SlangGlobalSessionDesc desc = {};
        createGlobalSession(&desc, globalSession.writeRef());
    }

    TargetDesc targetDesc;
    targetDesc.format = SlangCompileTarget::SLANG_WGSL;
    targetDesc.profile = globalSession->findProfile("sm_6_5");

    SessionDesc sessionDesc;
    sessionDesc.searchPaths = shaderSources.data();
    sessionDesc.searchPathCount = shaderSources.size();
    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;
    sessionDesc.flags = 0;

    std::vector<slang::CompilerOptionEntry> options;

    sessionDesc.compilerOptionEntries = options.data();
    sessionDesc.compilerOptionEntryCount = options.size();

    globalSession->createSession(sessionDesc, session.writeRef());
    Assert::Check(session != nullptr, "session != nullptr", "Error creating Shader Compiler session");

    slang::IModule* slangModule = nullptr;
    slangModule = session->loadModule(shader_name.c_str());

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

    ComPtr<IBlob> e;
    composedProgram->getTargetCode(0, e.writeRef());
    code += std::string(static_cast<const char*>(e->getBufferPointer()));

    std::cout << "Shader compile successful" << std::endl;

    wgpu::ShaderSourceWGSL shaderCodeDesc = wgpu::Default;
    shaderCodeDesc.chain.next = nullptr;
    shaderCodeDesc.chain.sType = wgpu::SType::ShaderSourceWGSL;
    shaderCodeDesc.code = {code.c_str(), code.length()};

    std::cout << code << std::endl;

    wgpu::ShaderModuleDescriptor shaderDesc = wgpu::Default;


    shaderDesc.nextInChain = &shaderCodeDesc.chain;
    shaderDesc.label = {shader_name.c_str(), shader_name.length()};
    auto device = Application::Get().GetDevice();
    auto module = device.createShaderModule(shaderDesc);

    Assert::Check(module != nullptr, "module != nullptr", "Error compiling module");
}
