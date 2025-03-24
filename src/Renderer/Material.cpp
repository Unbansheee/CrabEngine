module Engine.Resource.Material;
import Engine.GFX.MeshVertex;
import Engine.Resource.ShaderFile;
import Engine.Application;

void MaterialResource::Apply(wgpu::RenderPassEncoder renderPass)
{
    renderPass.setPipeline(GetPipeline());
    if (bBindGroupsDirty)
    {
        for (auto grp : m_bindGroups)
        {
            grp.BindGroup.release();
        }
        
        m_bindGroups = CreateMaterialBindGroups();
        bBindGroupsDirty = false;
    }
    for (auto& grp : m_bindGroups)
    {
        renderPass.setBindGroup(grp.BindGroupIndex, grp.BindGroup, 0, nullptr);
    }
}

void MaterialResource::LoadData()
{
    LoadFromShaderPath(Application::Get().GetDevice(), shader_file.Get<ShaderFileResource>()->shaderFilePath);
    Resource::LoadData();
    MarkBindGroupsDirty();
}

void MaterialResource::LoadFromShaderPath(wgpu::Device device, const std::filesystem::path& shaderPath,
                                          MaterialSettings settings)
{
    m_device = device;
    m_settings = settings;
    m_shaderModule = ResourceManager::loadShaderModule(shaderPath, device);
    Initialize();
    loaded = true;
}

void MaterialResource::OnPropertySet(Property& prop)
{
    Resource::OnPropertySet(prop);

    if ((prop.flags & PropertyFlags::MaterialProperty))
    {
        MarkBindGroupsDirty();
    }
}

wgpu::RenderPipeline MaterialResource::CreateRenderPipeline()
{
    Vertex::VertexBufferLayout layout;
    Vertex::CreateVertexBufferLayout<MeshVertex>(layout);
    
    wgpu::VertexBufferLayout vertexBufferLayout = layout.Layout;
    
    wgpu::RenderPipelineDescriptor pipelineDesc = {};
    pipelineDesc.vertex.bufferCount = 1;
    pipelineDesc.vertex.buffers = &vertexBufferLayout;
    pipelineDesc.vertex.entryPoint = "vs_main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;
    pipelineDesc.vertex.module = m_shaderModule;
    pipelineDesc.primitive.topology = m_settings.PrimitiveTopology;
    pipelineDesc.primitive.frontFace = m_settings.FrontFace;
    pipelineDesc.primitive.cullMode = m_settings.CullMode;
    pipelineDesc.multisample.count = 1;
    pipelineDesc.multisample.mask = ~0u;
    pipelineDesc.multisample.alphaToCoverageEnabled = false;

    wgpu::BlendState blendState;
    blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
    blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
    blendState.color.operation = wgpu::BlendOperation::Add;
	
    blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
    blendState.alpha.dstFactor = wgpu::BlendFactor::One;
    blendState.alpha.operation = wgpu::BlendOperation::Add;
    
    wgpu::ColorTargetState colorTarget;
    colorTarget.format = TargetTextureFormat;
    colorTarget.blend = &blendState;
    colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.
    
    wgpu::FragmentState fragmentState = {};
    fragmentState.module = m_shaderModule;
    fragmentState.entryPoint = "fs_main";
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    
    wgpu::DepthStencilState depthStencilState = wgpu::Default;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    depthStencilState.depthWriteEnabled = true;
    depthStencilState.format = DepthTextureFormat;
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    pipelineDesc.depthStencil = &depthStencilState;
    
    auto bindGroupLayouts = CreateMaterialBindGroupLayouts();
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)bindGroupLayouts.data();
    pipelineDesc.layout = m_device.createPipelineLayout(layoutDesc);
    

    return m_device.createRenderPipeline(pipelineDesc);
}
