//
// Created by Vinnie on 9/04/2025.
//

module;

#include "slang/include/slang-com-ptr.h"

export module Engine.ShaderCompiler;
import std;


export class ShaderCompiler {
public:
     ShaderCompiler(const std::string& shader_name);

     Slang::ComPtr<slang::ISession> session;
     static inline std::vector<const char*> shaderSources = {ENGINE_RESOURCE_DIR};

     std::string code;
};
