module;
#include "ReflectionMacros.h"
#include "Renderer/MaterialHelpers.h"

export module Engine.GFX.IDPassMaterial;
import Engine.Resource.Material;


export class IDPassMaterial : public MaterialResource
{
public:
    CRAB_CLASS(IDPassMaterial, MaterialResource)
    BEGIN_PROPERTIES
    END_PROPERTIES

    IDPassMaterial();
    
    IDPassMaterial(const wgpu::Device& device, const std::filesystem::path& shaderPath, const MaterialSettings& settings = MaterialSettings())
        : MaterialResource(device, shaderPath, settings)
          
    {
    }

    void Initialize() override;
    std::vector<wgpu::BindGroupLayout> CreateMaterialBindGroupLayouts() override;
    std::vector<MaterialBindGroup> CreateMaterialBindGroups() override;
    void CreateVertexBufferLayouts(std::vector<Vertex::VertexBufferLayout>& Layouts) override;
    void UpdateUniforms() override;


};
