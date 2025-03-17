#include "ReflectionMacros.h"

export module shader_file_resource;
import string;
import resource;

export class ShaderFileResource : public Resource
{
    CRAB_CLASS(ShaderFileResource, Resource)
    BEGIN_PROPERTIES
        ADD_PROPERTY("Shader File", shaderFilePath)
    END_PROPERTIES
    
    std::string shaderFilePath;
};
