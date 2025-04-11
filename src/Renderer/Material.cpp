module Engine.Resource.Material;
import Engine.GFX.MeshVertex;
import Engine.Resource.ShaderFile;
import Engine.Application;
import Engine.Resource.ShaderFile;
import Engine.ShaderCompiler;
import Engine.Resource.Texture;
import Engine.Resource.RuntimeTexture;


void MaterialResource::Apply(wgpu::RenderPassEncoder renderPass)
{
    renderPass.setPipeline(GetPipeline());

    UpdateBindGroups();
    for (auto& grp : m_bindGroups) {
        renderPass.setBindGroup(grp.first, grp.second, 0, nullptr);
    }
}

void MaterialResource::UpdateBindGroups() {
    for (auto& [group, dirty] : m_dirtyGroups) {
        if (!dirty) continue;
        std::vector<WGPUBindGroupEntry> entries;

        // Collect all bindings for this group
        for (auto& [uniformName, meta] : m_uniformMetadata) {
            if (meta.Group != group) continue;

            WGPUBindGroupEntry entry = {};
            entry.binding = meta.Location;

            if (meta.BindingType == Buffer) {
                auto& buffer = m_buffers.at(uniformName);
                entry.buffer = buffer.buffer;
                entry.offset = 0;
                entry.size = buffer.size;
            }
            else if (meta.BindingType == Texture) {
                auto& texture = m_textures.at(uniformName);
                entry.textureView = texture.texture->GetInternalTextureView();
            }
            else if (meta.BindingType == StorageTexture) {
                auto& texture = m_textures.at(uniformName);
                entry.textureView = texture.texture->GetInternalTextureView();
            }
            else if (meta.BindingType == Sampler) {
                auto sampler = m_samplers.at(uniformName);
                entry.sampler = *sampler.sampler;
            }

            entries.push_back(entry);
        }

        // Sort entries by binding index
        std::sort(entries.begin(), entries.end(),
            [](auto& a, auto& b) { return a.binding < b.binding; });

        // Create new bind group
        WGPUBindGroupDescriptor desc = {
            .layout = *m_bindGroupLayouts.at(group),
            .entryCount = entries.size(),
            .entries = entries.data()
        };

        if (m_bindGroups[group]) {
            wgpuBindGroupRelease(m_bindGroups[group]);
        }

        m_bindGroups[group] = wgpuDeviceCreateBindGroup(Application::Get().GetDevice(), &desc);
        m_dirtyGroups[group] = false;
    }
}

void MaterialResource::SetUniform(const std::string& uniformName, void* data, uint32_t size) {
    Assert::Check(m_buffers.contains(uniformName), "Buffers.contains(uniformName)", "Parameter does not exist");
    auto buff = m_buffers.at(uniformName);
    Application::Get().GetQueue().writeBuffer(buff.buffer, 0, data, size);
}


void MaterialResource::SetTexture(const std::string &uniformName, const std::shared_ptr<TextureResource>& texture) {
    if (m_uniformMetadata.contains(uniformName)) {
        auto& meta = m_uniformMetadata.at(uniformName);
        m_textures[uniformName] = {texture};
        m_dirtyGroups[meta.Group] = true;
    }
}

void MaterialResource::SetSampler(const std::string &uniformName, wgpu::raii::Sampler sampler) {
    if (m_uniformMetadata.contains(uniformName)) {
        auto& meta = m_uniformMetadata.at(uniformName);
        m_samplers[uniformName] = {sampler};
        m_dirtyGroups[meta.Group] = true;
    }
}

void MaterialResource::LoadData()
{
    LoadFromShaderPath(Application::Get().GetDevice(), shader_file.Get<ShaderFileResource>()->shaderFilePath);
    Resource::LoadData();
    //MarkBindGroupsDirty();
}

void MaterialResource::LoadFromShaderPath(wgpu::Device device, const std::filesystem::path& shaderPath,
                                          MaterialSettings settings)
{
    m_device = device;
    m_settings = settings;

    ShaderCompiler c("dumbTestShader");

    m_shaderModule = c.GetCompiledShaderModule();
    auto l = c.GetPipelineLayout();
    m_pipelineLayout = l.PipelineLayout;
    m_bindGroupLayouts = l.BindGroupLayouts;

    for (auto& entry : c.GetUniformMetadata()) {
        m_uniformMetadata.insert({entry.Name, entry});
    }

    for (auto& grp : m_bindGroupLayouts) {
        m_dirtyGroups.insert({grp.first, true});
    }

    InitializeProperties();
    Initialize();
    loaded = true;
}

void MaterialResource::InitializeProperties() {
    auto queue = Application::Get().GetQueue();
    auto device = Application::Get().GetDevice();

    // Create initial buffers based on reflection data
    for (auto& [uniformName, meta] : m_uniformMetadata) {
        if (meta.BindingType == Buffer) {
            WGPUBufferDescriptor desc = {
                .label = {meta.Name.c_str(), meta.Name.length()},
                .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
                .size = meta.SizeInBytes
            };
            BufferBinding binding {
                .buffer = wgpuDeviceCreateBuffer(device, &desc),
                .size = meta.SizeInBytes,
                .isDynamic = meta.IsDynamic
            };
            m_buffers.emplace(uniformName, binding);
        }

        if (meta.BindingType == Texture) {
            TextureBinding binding = {
                ResourceManager::Load<TextureResource>(ENGINE_RESOURCE_DIR"/null_texture_black.png")
            };
            m_textures.emplace(uniformName, binding);
        }

        if (meta.BindingType == Sampler) {
            wgpu::SamplerDescriptor samplerDesc;
            samplerDesc.addressModeU = wgpu::AddressMode::Repeat;
            samplerDesc.addressModeV = wgpu::AddressMode::Repeat;
            samplerDesc.addressModeW = wgpu::AddressMode::Repeat;
            samplerDesc.magFilter = wgpu::FilterMode::Linear;
            samplerDesc.minFilter = wgpu::FilterMode::Linear;
            samplerDesc.mipmapFilter = wgpu::MipmapFilterMode::Linear;
            samplerDesc.lodMinClamp = 0.0f;
            samplerDesc.lodMaxClamp = 8.0f;
            samplerDesc.compare = wgpu::CompareFunction::Undefined;
            samplerDesc.maxAnisotropy = 1;
            wgpu::raii::Sampler s = m_device.createSampler(samplerDesc);
            SamplerBinding binding = {
                 s
            };
            m_samplers.emplace(uniformName, binding);
        }

        if (meta.BindingType == StorageTexture) {
            auto storageTex = std::make_shared<RuntimeTextureResource>();
            wgpu::TextureDescriptor desc{};
            desc.format = meta.Format;
            desc.size = WGPUExtent3D{1,1,1};
            desc.usage = wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopyDst | wgpu::TextureUsage::CopySrc;
            desc.sampleCount = 1;
            desc.mipLevelCount = 1;
            storageTex->CreateBlankTexture(desc);
            TextureBinding binding = {
                storageTex
            };
            m_textures.emplace(uniformName, binding);

        }
    }
}


void MaterialResource::OnPropertySet(Property& prop)
{
    Resource::OnPropertySet(prop);

    if ((prop.flags & PropertyFlags::MaterialProperty))
    {
        //MarkBindGroupsDirty();
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
    pipelineDesc.vertex.entryPoint = {"vs_main", wgpu::STRLEN};
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants = nullptr;
    pipelineDesc.vertex.module = *m_shaderModule;
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
    if (m_settings.bUseBlending)
    {
        colorTarget.blend = &blendState;
    }
    colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.
    
    wgpu::FragmentState fragmentState = {};
    fragmentState.module = *m_shaderModule;
    fragmentState.entryPoint = {"fs_main", wgpu::STRLEN};
    fragmentState.constantCount = 0;
    fragmentState.constants = nullptr;
    fragmentState.targetCount = 1;
    fragmentState.targets = &colorTarget;
    pipelineDesc.fragment = &fragmentState;
    
    wgpu::DepthStencilState depthStencilState = wgpu::Default;
    depthStencilState.depthCompare = wgpu::CompareFunction::Less;
    depthStencilState.depthWriteEnabled = WGPUOptionalBool_True;
    depthStencilState.format = DepthTextureFormat;
    depthStencilState.stencilReadMask = 0;
    depthStencilState.stencilWriteMask = 0;
    pipelineDesc.depthStencil = &depthStencilState;
    
    //auto bindGroupLayouts = CreateMaterialBindGroupLayouts();
    //wgpu::PipelineLayoutDescriptor layoutDesc{};
    //layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
    //layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)bindGroupLayouts.data();
    pipelineDesc.layout = *m_pipelineLayout;


    

    return m_device.createRenderPipeline(pipelineDesc);
}
