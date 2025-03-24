#include "ReflectionMacros.h"

export module Engine.Resource.ShaderFile;
import std;
import Engine.Resource;

export class ShaderFileResource : public Resource
{
    CRAB_CLASS(ShaderFileResource, Resource)
    BEGIN_PROPERTIES
        ADD_PROPERTY("Shader File", shaderFilePath)
    END_PROPERTIES
    
    std::string shaderFilePath;
};
