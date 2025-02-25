#include <webgpu/webgpu.hpp>
#include <vector>
#include <unordered_map>
#include <algorithm>

#include "PipelineCache.h"
#include "RenderVisitor.h"
#include "Utility/MathUtils.h"


struct Camera
{
    Matrix4 GetViewProjMatrix() const
    {
        return Matrix4(1.0);
    }

    Vector3 GetPosition() const
    {
        return {0, 0, 0};
    };
};

class Renderer {
public:
    Renderer() = default;

    // Initialization
    void Initialize(wgpu::Device device, wgpu::Surface surface, uint32_t width, uint32_t height, wgpu::TextureFormat surfaceFormat, wgpu::TextureFormat depthFormat) {
        m_device = device;
        m_queue = device.getQueue();
        m_surface = surface;
        m_depthFormat = depthFormat;
        m_surfaceFormat = surfaceFormat;
        
        CreateSwapChain(width, height);
        CreateDepthTexture(width, height);


        wgpu::SupportedLimits supportedLimits;
        device.getLimits(&supportedLimits);
        wgpu::Limits deviceLimits = supportedLimits.limits;
        auto uniformStride = MathUtils::Align((uint32_t)sizeof(GlobalUniforms), (uint32_t)deviceLimits.minUniformBufferOffsetAlignment);

        wgpu::BufferDescriptor bufferDesc;
        bufferDesc.label = "Uniform Buffer";
        bufferDesc.size = uniformStride + sizeof(GlobalUniforms);
        bufferDesc.usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst;
        bufferDesc.mappedAtCreation = false;
        m_globalUniformBuffer = device.createBuffer(bufferDesc);

        // Create bind group for model buffer (created once during init)
        //m_modelBindGroup = CreateModelBindGroup(m_dynamicModelBuffer);
    }

    void RequestResize(uint32_t width, uint32_t height)
    {
        resizeRequest.active = true;
        resizeRequest.width = width;
        resizeRequest.height = height;
    }
    
    void Resize(uint32_t width, uint32_t height)
    {
        m_depthTexture.destroy();
        m_depthTexture.release();
        CreateDepthTexture(width, height);
        CreateSwapChain(width, height);
    }
    
    // Main rendering entry point
    void Render(Node* rootNode, const Camera& camera) {
        //m_dynamicModelBuffer.reset(new DynamicUniformBuffer(m_device, 1000));
        auto view = GetCurrentTextureView();

        // 1. Update all dynamic data
        UpdateGlobalUniforms(camera);
        
        // 2. Collect draw commands
        RenderVisitor visitor(m_device, *m_dynamicModelBuffer);
        
        rootNode->Render(visitor);
        
        // 3. Build and sort batches
        auto batches = visitor.BuildBatches();
        SortBatches(batches);

        
        // 4. Execute rendering
        ExecuteBatches(batches, view);
        m_queue.submit(m_additionalPasses.size(), m_additionalPasses.data());
        // Execute additional commands
        for (auto command : m_additionalPasses)
        {
            command.release();
        }
        m_additionalPasses.clear();
        
        // 5. Present frame
        m_surface.present();
        
        m_currentSurfaceView.release();
        m_currentSurfaceView = nullptr;

        if (resizeRequest.active)
        {
            Resize(resizeRequest.width, resizeRequest.height);
            resizeRequest = {};
        }
    }

    void AddCommand(wgpu::CommandBuffer command)
    {
        m_additionalPasses.push_back(command);
    }

    wgpu::TextureView GetCurrentTextureView();

    
private:
    // Core WebGPU objects
    wgpu::Device m_device = nullptr;
    wgpu::Queue m_queue = nullptr;
    wgpu::Surface m_surface = nullptr;
    wgpu::Texture m_depthTexture = nullptr;
    wgpu::TextureFormat m_surfaceFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat m_depthFormat = wgpu::TextureFormat::Undefined;
    std::vector<wgpu::CommandBuffer> m_additionalPasses;
    wgpu::TextureView m_currentSurfaceView = nullptr;

    
    // Rendering state
    wgpu::BindGroup m_globalBindGroup = nullptr;
    wgpu::Buffer m_globalUniformBuffer = nullptr;
    std::unique_ptr<DynamicUniformBuffer> m_dynamicModelBuffer = nullptr;
    //wgpu::BindGroup m_modelBindGroup = nullptr;
    
    // Frame state
    struct GlobalUniforms {
        glm::mat4 viewProj;
        glm::vec3 cameraPos;
        // ... other scene-wide uniforms
    };

    struct ResizeRequest
    {
        bool active = false;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    ResizeRequest resizeRequest;
    
    void CreateSwapChain(uint32_t width, uint32_t height) {
        if (m_currentSurfaceView) m_currentSurfaceView.release();
        m_currentSurfaceView = nullptr;
        wgpu::SurfaceConfiguration desc;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.format = m_surfaceFormat;
        desc.width = width;
        desc.height = height;
        desc.presentMode = wgpu::PresentMode::Fifo;
        desc.viewFormats = nullptr;
        desc.viewFormatCount = 0;
        desc.device = m_device;
        m_surface.configure(desc);
    }

    void CreateDepthTexture(uint32_t width, uint32_t height) {
        wgpu::TextureDescriptor desc;
        desc.size = {width, height, 1};
        desc.mipLevelCount = 1;
        desc.sampleCount = 1;
        desc.dimension = wgpu::TextureDimension::_2D;
        desc.format = m_depthFormat;
        desc.usage = wgpu::TextureUsage::RenderAttachment;
        desc.viewFormatCount = 1;
        desc.viewFormats = (WGPUTextureFormat*)&m_depthFormat;
       
        m_depthTexture = m_device.createTexture(desc);
    }

    void UpdateGlobalUniforms(const Camera& camera) {
        GlobalUniforms uniforms = {
            .viewProj = camera.GetViewProjMatrix(),
            .cameraPos = camera.GetPosition()
        };
        //m_queue.writeBuffer(m_globalUniformBuffer, 0, &uniforms, sizeof(GlobalUniforms));
    }

    void SortBatches(std::vector<DrawBatch>& batches) {
        // Sort criteria:
        // 1. Render pass priority (e.g., shadows first)
        // 2. Pipeline state
        // 3. Material properties
        /*
        std::sort(batches.begin(), batches.end(), [](const DrawBatch& a, const DrawBatch& b) {
            return std::tie(a.pass, a.pipeline, a.materialHash) < 
                   std::tie(b.pass, b.pipeline, b.materialHash);
        });
        */
    }
    
    
    void ExecuteBatches(const std::vector<DrawBatch>& batches, wgpu::TextureView colorAttachmentView) {
        // 1. Begin frame
        // Color attachment
        wgpu::RenderPassColorAttachment colorAttachment;
        colorAttachment.view = colorAttachmentView;
        colorAttachment.loadOp = wgpu::LoadOp::Clear;
        colorAttachment.storeOp = wgpu::StoreOp::Store;
        colorAttachment.clearValue = {0.05f, 0.05f, 0.05f, 1.0f};
        colorAttachment.resolveTarget = nullptr;
        
        // Depth attachment
        wgpu::TextureViewDescriptor depthTextureViewDesc;
        depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
        depthTextureViewDesc.baseArrayLayer = 0;
        depthTextureViewDesc.arrayLayerCount = 1;
        depthTextureViewDesc.baseMipLevel = 0;
        depthTextureViewDesc.mipLevelCount = 1;
        depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
        depthTextureViewDesc.format = m_depthFormat;
        WGPUTextureView depth_view = wgpuTextureCreateView(m_depthTexture, &depthTextureViewDesc);
        wgpu::RenderPassDepthStencilAttachment depthAttachment;
        depthAttachment.view = depth_view;
        depthAttachment.depthLoadOp = wgpu::LoadOp::Clear;
        depthAttachment.depthStoreOp = wgpu::StoreOp::Store;
        depthAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
        depthAttachment.stencilStoreOp = wgpu::StoreOp::Store;
        depthAttachment.depthClearValue = 1.0f;
        
        // Assemble render pass
        wgpu::RenderPassDescriptor desc;
        desc.colorAttachmentCount = 1;
        desc.colorAttachments = &colorAttachment;
        desc.depthStencilAttachment = &depthAttachment;

        
        wgpu::CommandEncoder encoder = m_device.createCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.beginRenderPass(desc);
        
        // 2. Track current state
        Material* currentMaterial = nullptr;
        
        for (const auto& batch : batches) {
            // Pipeline state change
            if (batch.material != currentMaterial) {
                currentMaterial = batch.material;
                currentMaterial->Apply(pass);
            }
            
            // Execute draw calls
            for (const auto& item : batch.drawItems) {
                uint32_t dynamicOffset = item.dynamicOffset;
                
                pass.setVertexBuffer(0, item.vertexBuffer, 0, WGPU_WHOLE_SIZE);

                // Model bind group
                //pass.setBindGroup(0, m_modelBindGroup, 1, &dynamicOffset);
                
                if (item.indexCount > 0)
                {
                    pass.setIndexBuffer(item.indexBuffer, wgpu::IndexFormat::Uint16, 0, WGPU_WHOLE_SIZE);
                    pass.drawIndexed(
                        item.indexCount,
                        1,  // instanceCount
                        0,  // firstIndex
                        0,  // baseVertex
                        0   // firstInstance
                    );
                }
                else
                {
                    pass.draw(
                        item.vertexCount,
                        1, // instanceCount
                        0, // firstVertex
                        0); // firstInstance
                }
            }
        }
        
        // 3. End frame
        pass.end();
        wgpu::CommandBuffer commands = encoder.finish();
        m_queue.submit(1, &commands);

        pass.release();
        commands.release();
        encoder.release();
    }

    wgpu::TextureView GetNextSurfaceTextureView() const;
};
