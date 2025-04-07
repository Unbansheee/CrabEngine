
export module Engine.Compute.ClearTexture;
import std;
import Engine.Resource.Texture;
import Engine.WGPU;
import Engine.Application;
import Engine.Resource.ResourceManager;
import fmt;

export template<WGPUTextureFormat TextureFormat, WGPUStorageTextureAccess Access>
class ComputeClearTexture
{
    wgpu::ComputePipeline computePipeline = nullptr;
    wgpu::BindGroupLayout bindGroupLayout = nullptr;

public:
    void Execute(std::shared_ptr<TextureResource> texture)
    {
        auto device = Application::Get().GetDevice();
        auto queue = Application::Get().GetQueue();
        
        // Create compute bind group
        std::vector<wgpu::BindGroupEntry> entries(1, wgpu::Default);

        // Input buffer
        entries[0].binding = 0;
        entries[0].textureView = texture->GetInternalTextureView();
        entries[0].offset = 0;

        wgpu::BindGroupDescriptor bindGroupDesc;
        bindGroupDesc.layout = bindGroupLayout;
        bindGroupDesc.entryCount = (uint32_t)entries.size();
        bindGroupDesc.entries = (WGPUBindGroupEntry*)entries.data();
        auto bindGroup = device.createBindGroup(bindGroupDesc);
        
        auto encoder = device.createCommandEncoder();
        
        wgpu::ComputePassDescriptor computePassDesc = wgpu::Default;
        computePassDesc.timestampWrites = nullptr;
        wgpu::ComputePassEncoder computePass = encoder.beginComputePass(computePassDesc);

        uint32_t width = (texture->GetSize().x);
        uint32_t height = (texture->GetSize().y);

        uint32_t workGroupSize = 32;
        uint32_t workGroupCount_x = (width + workGroupSize-1) / workGroupSize;
        uint32_t workGroupCount_y = (height + workGroupSize-1) / workGroupSize;

        computePass.setPipeline(computePipeline);
        computePass.setBindGroup(0, bindGroup, 0, nullptr);
        computePass.dispatchWorkgroups(workGroupCount_x,workGroupCount_y,1);

        computePass.end();
        queue.submit(encoder.finish());
        
#if !defined(WEBGPU_BACKEND_WGPU)
        wgpuComputePassEncoderRelease(computePass);
#endif
    }
    
    ComputeClearTexture()
    {
        auto device = Application::Get().GetDevice();
        auto queue = Application::Get().GetQueue();


        std::unordered_map<std::string, std::string> formats {
        {"#ACCESS", MapAccessToString()},
        {"#FORMAT", MapFormatToString()}
        };
        wgpu::ShaderModule computeShaderModule = ResourceManager::loadComputeShaderModule(ENGINE_RESOURCE_DIR "/Compute/clear_texture.wsl", device, formats);
        
        // Create compute pipeline
        wgpu::ComputePipelineDescriptor computePipelineDesc = wgpu::Default;
        computePipelineDesc.compute.entryPoint = "clearTexture";
        computePipelineDesc.compute.module = computeShaderModule;

        wgpu::StorageTextureBindingLayout t;
        t.format = TextureFormat;
        t.viewDimension = WGPUTextureViewDimension_2D;
        t.access = Access;
        
        std::vector<wgpu::BindGroupLayoutEntry> bindings(1, wgpu::Default);
        bindings[0].binding = 0;
        bindings[0].storageTexture = t;
        bindings[0].visibility = wgpu::ShaderStage::Compute;

        wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc;
        bindGroupLayoutDesc.entryCount = (uint32_t)bindings.size();
        bindGroupLayoutDesc.entries = bindings.data();
        bindGroupLayout = device.createBindGroupLayout(bindGroupLayoutDesc);

        wgpu::PipelineLayoutDescriptor pipelineLayoutDesc;
        pipelineLayoutDesc.bindGroupLayoutCount = 1;
        pipelineLayoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&bindGroupLayout;
        auto pipelineLayout = device.createPipelineLayout(pipelineLayoutDesc);
        computePipelineDesc.layout = pipelineLayout;
        
        computePipeline = device.createComputePipeline(computePipelineDesc);
    }

    constexpr std::string MapAccessToString()
    {
        switch (Access)
        {
        case WGPUStorageTextureAccess_Undefined:
            return "INVALID";
            break;
        case WGPUStorageTextureAccess_WriteOnly:
            return "write";
            break;
        case WGPUStorageTextureAccess_ReadOnly:
            return "read";
            break;
        case WGPUStorageTextureAccess_ReadWrite:
            return "read_write";
            break;
        case WGPUStorageTextureAccess_Force32:
            return "force32";
            break;
        }

        return "";
    }

    constexpr std::string MapFormatToString()
    {
        switch (TextureFormat)
        {
        case WGPUTextureFormat_R32Uint:
            return "r32uint";
        case WGPUTextureFormat_RGBA8Snorm:
            return "rgba8snorm";
        case WGPUTextureFormat_RGBA8Unorm:
            return "rgba8unorm";
        case WGPUTextureFormat_RGBA8UnormSrgb:
            return "rgba8unormsrgb";
        case WGPUTextureFormat_RGBA8Sint:
            return "rgba8sint";
        case WGPUTextureFormat_RGBA8Uint:
            return "rgba8uint";
        }

        //Assert::Check()
        //static_assert(false, "Texture format not implemented");
        
        return "";
    }
};