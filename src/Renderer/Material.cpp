module Engine.Resource.Material;
import Engine.GFX.MeshVertex;
import Engine.Resource.ShaderFile;
import Engine.Application;
import Engine.Resource.ShaderFile;
import Engine.ShaderCompiler;
import Engine.Resource.Texture;
import Engine.Resource.RuntimeTexture;
import Engine.JsonConversions;

std::vector<std::string> split_dotted_path(const std::string& path) {
    std::vector<std::string> components;
    size_t start = 0;
    size_t end = 0;

    while ((end = path.find('.', start)) != std::string::npos) {
        components.push_back(path.substr(start, end - start));
        start = end + 1;
    }

    // Add the last component after the final dot
    components.push_back(path.substr(start));

    return components;
}

void MaterialResource::Apply(wgpu::RenderPassEncoder renderPass)
{
    renderPass.setPipeline(*GetPipeline());

    UpdateBindGroups();
    for (auto& grp : m_bindGroups) {
        renderPass.setBindGroup(grp.first, *grp.second, 0, nullptr);
    }
}

void MaterialResource::Serialize(nlohmann::json &archive) {
    Resource::Serialize(archive);

    auto& uniforms = archive["uniforms"];
    for (auto uniform : m_uniformMetadata) {
        if (uniform.second.IsPushConstant) continue;
        if (!uniform.second.IsSerialized) continue;
        auto& current = uniforms[uniform.first];
        current["type"] = uniform.second.BindingType;
        switch (uniform.second.BindingType) {
            case Buffer: {
                auto b = m_cpuBuffers.at(uniform.first);
                auto buffer_data = b.data;
                int currentOffset = 0;
                for (auto& field : uniform.second.BufferFields) {
                    std::vector<uint8_t> bufferSection = std::vector(buffer_data.begin() + currentOffset, buffer_data.begin() + currentOffset + field.SizeInBytes);
                    if (field.Type == UniformMetadata::BufferField::FLOAT) current[field.Name] = *reinterpret_cast<float*>(bufferSection.data());
                    if (field.Type == UniformMetadata::BufferField::INT) current[field.Name] = *reinterpret_cast<int*>(bufferSection.data());
                    if (field.Type == UniformMetadata::BufferField::VECTOR2) current[field.Name] = *reinterpret_cast<glm::vec2*>(bufferSection.data());
                    if (field.Type == UniformMetadata::BufferField::VECTOR3) current[field.Name] = *reinterpret_cast<glm::vec3*>(bufferSection.data());
                    if (field.Type == UniformMetadata::BufferField::VECTOR4) current[field.Name] = *reinterpret_cast<glm::vec4*>(bufferSection.data());
                    currentOffset += field.SizeInBytes;
                }
            }
            break;
            case Texture: {
                auto& binding = m_textures.at(uniform.first);
                auto& val = binding.texture;
                if (val)
                {
                    if (val->IsInline())
                    {
                        current["import_type"] = "inline";
                        auto& inlineDef = current["inline_resource"];
                        val->Serialize(inlineDef);
                    }
                    else {
                        current["import_type"] = "file";
                        current["source_file_path"] = val->GetSourcePath();
                        ResourceManager::SaveResource(val);
                    }
                }
                else
                {
                    current["import_type"] = "";
                    current["source_file_path"] = "";
                }
            }
            break;



            default: break;
        }
    }
}

void MaterialResource::Deserialize(nlohmann::json &archive) {
    Resource::Deserialize(archive);

    LoadFromShaderPath(Application::Get().GetDevice(), "ghghg");

    if (archive.contains("uniforms")) {
        nlohmann::json& uniformJson = archive.at("uniforms");

    for (auto& [uniformName, uniform] : m_uniformMetadata) {
        if (!uniform.IsSerialized) continue;
        if (!uniformJson.contains(uniformName)) continue;

        if (uniform.BindingType == Buffer) {
            auto& b = m_cpuBuffers.at(uniformName);
            for (auto& field : uniform.BufferFields) {
                if (!uniformJson.at(uniformName).contains(field.Name)) { continue; }
                nlohmann::json& fieldJson = uniformJson.at(uniformName).at(field.Name);
                if (field.Type == UniformMetadata::BufferField::FLOAT) {
                    float data = fieldJson.get<float>();
                    SetUniformData(uniformName, &data, field.SizeInBytes, field.Offset);
                };
                if (field.Type == UniformMetadata::BufferField::VECTOR2) {
                    glm::vec2 data = fieldJson.get<glm::vec2>();
                    SetUniformData(uniformName, &data, field.SizeInBytes, field.Offset);
                };
                if (field.Type == UniformMetadata::BufferField::VECTOR3) {
                    glm::vec3 data = fieldJson.get<glm::vec3>();
                    SetUniformData(uniformName, &data, field.SizeInBytes, field.Offset);
                };
                if (field.Type == UniformMetadata::BufferField::VECTOR4) {
                    glm::vec4 data = fieldJson.get<glm::vec4>();
                    SetUniformData(uniformName, &data, field.SizeInBytes, field.Offset);
                };
            }
        }

        else if (uniform.BindingType == Texture) {
            auto& current = uniformJson.at(uniformName);
            auto& b = m_textures.at(uniformName);
            bool isInline = current.at("import_type").get<std::string>() == "inline";
            bool isSource = current.at("import_type").get<std::string>() == "file";
            if (!isInline && !isSource) continue;

            if (isInline)
            {
                auto& j = current["inline_resource"];
                auto res = static_cast<Resource*>(ClassDB::Get().GetClassByName(j.at("class"))->Initializer());
                std::shared_ptr<Resource> resource;
                resource.reset(res);
                resource->Deserialize(j);
                auto texture = std::static_pointer_cast<TextureResource>(resource);
                SetTexture(uniformName, texture);
            }
            else {
                std::string source_path = current.at("source_file_path").get<std::string>();
                auto texture = ResourceManager::Load<TextureResource>(source_path);
                SetTexture(uniformName, texture);
            }
        }
    }
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
                entry.buffer = *buffer.buffer;
                entry.offset = 0;
                entry.size = buffer.size;
            }
            else if (meta.BindingType == Texture) {
                auto& texture = m_textures.at(uniformName);
                entry.textureView = *texture.texture->GetInternalTextureView();
            }
            else if (meta.BindingType == StorageTexture) {
                auto& texture = m_textures.at(uniformName);
                entry.textureView = *texture.texture->GetInternalTextureView();
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


        m_bindGroups[group] = {wgpuDeviceCreateBindGroup(Application::Get().GetDevice(), &desc)};
        m_dirtyGroups[group] = false;
    }

    for (auto& [name, cpuBuffer] : m_cpuBuffers) {
        if (cpuBuffer.isDirty) {
            wgpu::raii::Buffer& gpuBuffer = m_buffers[name].buffer;
            Application::Get().GetQueue().writeBuffer(*gpuBuffer, 0, cpuBuffer.data.data(), cpuBuffer.size);
            cpuBuffer.isDirty = false;
        }
    }
}

wgpu::raii::TextureView MaterialResource::GetThumbnail() {
    if (!MaterialResourceThumbnail) {
        MaterialResourceThumbnail = ResourceManager::Load<TextureResource>("/engine/Textures/T_MaterialThumbnail.png");
    }
    return MaterialResourceThumbnail->GetInternalTextureView();
}

std::vector<uint8_t> MaterialResource::GetUniformData(const std::string &name) {
    return m_cpuBuffers.at(name).data;
}

void MaterialResource::SetUniformData(const std::string& uniformName, void* data, uint32_t size, uint32_t offset) {
    auto path = split_dotted_path(uniformName);
    if (path.size() == 1) {
        Assert::Check(m_cpuBuffers.contains(path[0]), "Buffers.contains(uniformName)", "Parameter does not exist");
        auto& buff = m_cpuBuffers.at(path[0]);
        auto meta = buff.uniformMetadata;
        memcpy(buff.data.data() + offset, data, size);
        buff.isDirty = true;
    }

    else {
        Assert::Check(m_cpuBuffers.contains(path[0]), "Buffers.contains(uniformName)", "Parameter does not exist");
        auto& buff = m_cpuBuffers.at(path[0]);
        auto meta = buff.uniformMetadata;

        Assert::Fail("Not Implemented");
    }
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

std::shared_ptr<TextureResource> MaterialResource::GetTexture(const std::string &uniformName) {
    return m_textures.at(uniformName).texture;
}

std::vector<uint8_t> MaterialResource::ReadBufferData(const wgpu::raii::Buffer &buffer, uint32_t size) {

    wgpu::BufferDescriptor desc;
    desc.size = size;
    desc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;

    wgpu::raii::Buffer currentReadbackBuffer = Application::Get().GetDevice().createBuffer(desc);

    auto enc = Application::Get().GetDevice().createCommandEncoder();
    enc.copyBufferToBuffer(*buffer, 0, *currentReadbackBuffer, 0, size);
    wgpu::raii::CommandBuffer q = enc.finish();
    Application::Get().GetQueue().submit(*q);


    wgpu::BufferMapCallbackInfo info;
    info.callback = [](WGPUMapAsyncStatus wgpu_map_async_status, WGPUStringView wgpu_string_view, void * p, void * p2) {  };
    auto f = currentReadbackBuffer->mapAsync(wgpu::MapMode::Read, 0, size, info);

    Application::Get().GetDevice().poll(true, nullptr);


    const uint8_t* data = (uint8_t*)currentReadbackBuffer->getConstMappedRange(0, size);

    return std::vector<uint8_t>(data, data + size);
}

void MaterialResource::CreateBuffer(const std::string &name, uint32_t size, bool isDynamic) {
    auto device = Application::Get().GetDevice();

    WGPUBufferDescriptor desc = {
        .label = {name.c_str(), name.length()},
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::Uniform,
        .size = size
    };

    BufferBinding binding {
        .buffer = wgpuDeviceCreateBuffer(device, &desc),
        .size = size,
        .isDynamic = isDynamic
    };

    m_buffers[name] =  binding;
    // Initialize CPU mirror

    m_cpuBuffers[name] = CPUBuffer{
        std::vector<uint8_t>(size, 0),
        true,
        size,
        m_uniformMetadata.at(name)
    };
}

void MaterialResource::LoadData()
{
    Resource::LoadData();
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
            if (!m_cpuBuffers.contains(uniformName) || !m_buffers.contains(uniformName)) {
                CreateBuffer(uniformName, meta.SizeInBytes, meta.IsDynamic);
            }
        }

        if (meta.BindingType == Texture) {
            TextureBinding binding = {
                ResourceManager::Load<TextureResource>("/engine/null_texture_black.png")
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

wgpu::raii::RenderPipeline MaterialResource::CreateRenderPipeline()
{
    Vertex::VertexBufferLayout layout;
    Vertex::CreateVertexBufferLayout<MeshVertex>(layout);
    
    wgpu::VertexBufferLayout vertexBufferLayout = layout.Layout;

    wgpu::RenderPipelineDescriptor pipelineDesc = wgpu::Default;
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
