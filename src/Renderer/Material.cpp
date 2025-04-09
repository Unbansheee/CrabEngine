module Engine.Resource.Material;
import Engine.GFX.MeshVertex;
import Engine.Resource.ShaderFile;
import Engine.Application;
import Engine.Resource.ShaderFile;

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

// Helper to get texture index in property order
uint32_t MaterialResource::GetTextureIndex(const std::string& name) const {
    size_t index = 0;
    for (const auto& [propName, prop] : m_metadata.Properties) {
        if (prop.Type == MaterialPropertyType::Texture2D) {
            if (propName == name) return index;
            index++;
        }
    }
    Assert::Fail("Texture property not found: " + name);
}

uint32_t MaterialResource::GetTextureBindingIndex(uint32_t textureIndex) const
{
    size_t index = 0;
    for (const auto& [propName, prop] : m_metadata.Properties) {
        if (prop.Type == MaterialPropertyType::Texture2D) {
            if (index == textureIndex) return index;
            index++;
        }
    }
    Assert::Fail("No texture binding found at index" + textureIndex);
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
    auto ret = ResourceManager::loadShaderModule(shaderPath, device);
    m_shaderModule = ret.Module;
    m_metadata = ret.Metadata;
    InitializeProperties();
    Initialize();
    loaded = true;
}

void MaterialResource::InitializeProperties() {
    std::vector<wgpu::BindGroupLayoutEntry> layoutEntries;
    std::vector<wgpu::BindGroupEntry> bindGroupEntries;
    std::vector<uint8_t> uniformData;
    std::unordered_map<std::string, PropertyLayoutInfo> layoutInfo;

    auto queue = Application::Get().GetQueue();
    auto device = Application::Get().GetDevice();


    // First pass: Calculate uniform buffer layout
    size_t currentOffset = 0;
    for (const auto& [name, prop] : m_metadata.Properties) {
        if (prop.Type == MaterialPropertyType::Texture2D) continue;

        const size_t alignment = WGSLAligner::AlignOf(prop.Type);
        currentOffset = (currentOffset + alignment - 1) / alignment * alignment;

        layoutInfo[name] = {
            .Offset = currentOffset,
            .Size = WGSLAligner::SizeOf(prop.Type)
        };

        currentOffset += layoutInfo[name].Size;
    }

    // Create uniform buffer
    const size_t bufferSize = (currentOffset + 15) / 16 * 16; // Round up to 16 bytes
    uniformData.resize(bufferSize);

    // Second pass: Initialize default values
    for (const auto& [name, prop] : m_metadata.Properties) {
        if (prop.Type == MaterialPropertyType::Texture2D) continue;

        if (!prop.DefaultValue.has_value()) continue;
        const auto& info = layoutInfo[name];
        switch(prop.Type) {
            case MaterialPropertyType::Float:
                *reinterpret_cast<float*>(uniformData.data() + info.Offset) =
                    std::any_cast<float>(prop.DefaultValue);
                break;
            case MaterialPropertyType::Int:
                *reinterpret_cast<int*>(uniformData.data() + info.Offset) =
                    std::any_cast<int>(prop.DefaultValue);
            break;
            case MaterialPropertyType::UInt:
                *reinterpret_cast<uint32_t*>(uniformData.data() + info.Offset) =
                    std::any_cast<uint32_t>(prop.DefaultValue);
            break;
            case MaterialPropertyType::Vector2:
                *reinterpret_cast<glm::vec2*>(uniformData.data() + info.Offset) =
                    std::any_cast<glm::vec2>(prop.DefaultValue);
            break;
            case MaterialPropertyType::Vector3:
                *reinterpret_cast<glm::vec3*>(uniformData.data() + info.Offset) =
                    std::any_cast<glm::vec3>(prop.DefaultValue);
            break;
            case MaterialPropertyType::Vector4:
                *reinterpret_cast<glm::vec4*>(uniformData.data() + info.Offset) =
                    std::any_cast<glm::vec4>(prop.DefaultValue);
            break;
            default: break;
        }
    }

    // Create GPU buffer
    WGPUBufferDescriptor bufferDesc{
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform,
        .size = bufferSize
    };

    uniformBuffer = device.createBuffer(bufferDesc);
    queue.writeBuffer(uniformBuffer, 0, uniformData.data(), bufferSize);

    // Create texture resources
    std::vector<wgpu::TextureView> textureViews;
    uint32_t bindingIndex = 0;

    // Add uniform buffer to bind group
    if (bufferSize > 0) {
        wgpu::BufferBindingLayout b;
        b.type = wgpu::BufferBindingType::Uniform;

        wgpu::BindGroupLayoutEntry e;
        e.binding = bindingIndex;
        e.visibility = wgpu::ShaderStage::Fragment;
        e.buffer = b;

        layoutEntries.push_back(e);

        wgpu::BindGroupEntry groupEntry;
        groupEntry.binding = bindingIndex++;
        groupEntry.buffer = uniformBuffer;
        groupEntry.size = bufferSize;

        bindGroupEntries.push_back(groupEntry);
    }

    // Process textures
    for (const auto& [name, prop] : m_metadata.Properties) {
        if (prop.Type != MaterialPropertyType::Texture2D) continue;

        // Create default white texture
        WGPUTextureDescriptor texDesc{
            .usage = wgpu::TextureUsage::TextureBinding,
            .size = {1, 1},
            .format = wgpu::TextureFormat::RGBA8Unorm,
        };
        wgpu::Texture texture = device.createTexture(texDesc);

        // Initialize with white pixel
        const uint32_t white = 0xFFFFFFFF;
        queue.writeTexture(
            WGPUTexelCopyTextureInfo{.texture = texture},
            &white,
            sizeof(uint32_t),
            WGPUTexelCopyBufferLayout{.bytesPerRow = 4},
            WGPUExtent3D{1, 1}
        );

        textureViews.push_back(texture.createView());

        // Add texture and sampler to bind group
        layoutEntries.push_back(WGPUBindGroupLayoutEntry{
            .binding = bindingIndex,
            .visibility = wgpu::ShaderStage::Fragment,
            .texture = WGPUTextureBindingLayout{
                .sampleType = wgpu::TextureSampleType::Float
            }
        });

        bindGroupEntries.push_back(WGPUBindGroupEntry{
            .binding = bindingIndex++,
            .textureView = textureViews.back()
        });

        layoutEntries.push_back(WGPUBindGroupLayoutEntry{
            .binding = bindingIndex,
            .visibility = wgpu::ShaderStage::Fragment,
            .sampler = WGPUSamplerBindingLayout{
                .type = wgpu::SamplerBindingType::Filtering
            }
        });

        wgpu::SamplerDescriptor samplerDesc{};
        bindGroupEntries.push_back(WGPUBindGroupEntry{
            .binding = bindingIndex++,
            .sampler = device.createSampler(samplerDesc)
        });
    }

    // Create bind group layout
    WGPUBindGroupLayoutDescriptor layoutDesc{
        .entryCount = layoutEntries.size(),
        .entries = layoutEntries.data(),
    };
    bindGroupLayout = device.createBindGroupLayout(layoutDesc);

    // Create bind group
    WGPUBindGroupDescriptor bindGroupDesc{
        .layout = bindGroupLayout,
        .entryCount = bindGroupEntries.size(),
        .entries = bindGroupEntries.data(),
    };
    bindGroup = device.createBindGroup(bindGroupDesc);

    // Store layout information
    m_uniformData = std::move(uniformData);
    m_layoutInfo = std::move(layoutInfo);
    m_textureViews = std::move(textureViews);
    m_bindGroupEntries = std::move(bindGroupEntries);
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
    pipelineDesc.vertex.entryPoint = {"vs_main", wgpu::STRLEN};
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
    if (m_settings.bUseBlending)
    {
        colorTarget.blend = &blendState;
    }
    colorTarget.writeMask = wgpu::ColorWriteMask::All; // We could write to only some of the color channels.
    
    wgpu::FragmentState fragmentState = {};
    fragmentState.module = m_shaderModule;
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
    
    auto bindGroupLayouts = CreateMaterialBindGroupLayouts();
    wgpu::PipelineLayoutDescriptor layoutDesc{};
    layoutDesc.bindGroupLayoutCount = bindGroupLayouts.size();
    layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)bindGroupLayouts.data();
    pipelineDesc.layout = m_device.createPipelineLayout(layoutDesc);
    

    return m_device.createRenderPipeline(pipelineDesc);
}
