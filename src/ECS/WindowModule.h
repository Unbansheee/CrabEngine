#pragma once
#include <glfw3webgpu/glfw3webgpu.h>

#include "Components.h"
//#include "NodeImGUIContextWindow.h"
#include "Systems.h"

struct WindowRenderTargetComponent
{
    WGPUTextureFormat SurfaceFormat = wgpu::TextureFormat::BGRA8Unorm;
    wgpu::Surface Surface = nullptr;
    WGPUTextureFormat DepthFormat = wgpu::TextureFormat::Depth24Plus;
    wgpu::Texture DepthTexture = nullptr;
    wgpu::SurfaceTexture SurfaceTexture;
};

struct RenderTargetViewComponent
{
    wgpu::TextureView ColorView = nullptr;
    wgpu::TextureView DepthView = nullptr;
};

struct WindowComponent
{
    GLFWwindow* WindowPtr;
};

struct WindowModule
{
    struct WindowUserData {
        flecs::world* world; // Reference to the ECS world
        ecs_entity_t entity; // ID of the entity owning the window
    };
    
    WindowModule(flecs::world& w)
    {
        if (!glfwInit()) {
            std::cerr << "Could not initialize GLFW!" << std::endl;
        }

        flecs::world* win = &w;

        w.observer<WindowComponent>().event(flecs::OnAdd).each([win](flecs::entity e, WindowComponent& window)
        {
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
            window.WindowPtr = glfwCreateWindow(640, 480, e.name(), nullptr, nullptr);
            std::cout << "Window: " << window.WindowPtr << std::endl;
            if (!window.WindowPtr) {
                std::cerr << "Could not create window!" << std::endl;
                return;
            }


            WindowUserData* userData = new WindowUserData();
            userData->world = win;
            userData->entity = e.id();

            glfwSetWindowUserPointer(window.WindowPtr, userData );
            glfwSetFramebufferSizeCallback(window.WindowPtr, [](GLFWwindow* windowPtr, int x, int y)
            {
                WindowUserData* data = static_cast<WindowUserData*>(glfwGetWindowUserPointer(windowPtr));
                if (data && data->world->is_valid(data->entity)) {
                data->world->entity(data->entity).set<WindowResizeFlag>({(uint32_t) x, (uint32_t) y});
            }
            });
            
            e.set<WindowResizeFlag>({.x= 640, .y= 480});
        });

        w.observer<WindowComponent>().event(flecs::OnRemove).each([](flecs::entity e, WindowComponent& window)
        {
            WindowUserData* data = static_cast<WindowUserData*>(glfwGetWindowUserPointer(window.WindowPtr));
            glfwDestroyWindow(window.WindowPtr);
            delete data;
        });

        
        w.system<GPU, WindowComponent, WindowRenderTargetComponent, WindowResizeFlag>().kind(flecs::OnUpdate).term_at(0).singleton().each([](flecs::entity e, GPU& gpu, WindowComponent& window, WindowRenderTargetComponent& rt, WindowResizeFlag& resize)
        {
            rt.Surface.unconfigure();
            
            // reconfigure swapchain
            wgpu::SurfaceConfiguration desc;
            desc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;
            desc.format = rt.SurfaceFormat;
            desc.width = resize.x;
            desc.height = resize.y;
            desc.presentMode = wgpu::PresentMode::Immediate;
            desc.viewFormats = nullptr;
            desc.viewFormatCount = 0;
            desc.device = gpu.Device;
            rt.Surface.configure(desc);
            std::cout << "Resizing Window\n"; 
            
            // rebuild depth
            if (rt.DepthTexture)
            {
                rt.DepthTexture.destroy();
                rt.DepthTexture.release();
            }

            if (rt.DepthFormat != wgpu::TextureFormat::Undefined)
            {
                wgpu::TextureDescriptor desc;
                desc.size = {resize.x, resize.y, 1};
                desc.mipLevelCount = 1;
                desc.sampleCount = 1;
                desc.dimension = wgpu::TextureDimension::_2D;
                desc.format = rt.DepthFormat;
                desc.usage = wgpu::TextureUsage::RenderAttachment;
                desc.viewFormatCount = 1;
                desc.viewFormats = (WGPUTextureFormat*)&rt.DepthFormat;
                       
                rt.DepthTexture = gpu.Device.createTexture(desc);
            }
            e.remove<WindowResizeFlag>();
        });

        w.observer<GPU, WindowRenderTargetComponent, WindowComponent>().event(flecs::OnAdd).term_at(0).singleton().each([](flecs::entity e, GPU& gpu, WindowRenderTargetComponent& target, WindowComponent& window)
        {
            std::cout << "Render Target Window created\n";
            wgpu::Surface surface = glfwGetWGPUSurface(gpu.Instance, window.WindowPtr);
            wgpu::RequestAdapterOptions adapterOpts = {};
            adapterOpts.compatibleSurface = surface;
            wgpu::Adapter adapter = gpu.Instance.requestAdapter(adapterOpts);
            #ifdef WEBGPU_BACKEND_WGPU
                target.SurfaceFormat = surface.getPreferredFormat(adapter);
            #else
                target.SurfaceFormat = TextureFormat::BGRA8Unorm;
            #endif

            target.Surface = surface;
        });
        
        w.system<WindowRenderTargetComponent, RenderTargetViewComponent>().each([](WindowRenderTargetComponent& target, RenderTargetViewComponent& view)
        {
            if (view.ColorView != nullptr) view.ColorView.release();
            view.ColorView = nullptr;
            // Get the surface texture
            target.Surface.getCurrentTexture(&target.SurfaceTexture);
            if (target.SurfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success)
            {
                std::cout << "Bad surface get\n";
                return;
            }
            
            wgpu::Texture texture = target.SurfaceTexture.texture;

            // Create a view for this surface texture
            wgpu::TextureViewDescriptor viewDescriptor;
            viewDescriptor.label = "Surface texture view";
            viewDescriptor.format = target.SurfaceFormat;
            viewDescriptor.dimension = wgpu::TextureViewDimension::_2D;
            viewDescriptor.baseMipLevel = 0;
            viewDescriptor.mipLevelCount = 1;
            viewDescriptor.baseArrayLayer = 0;
            viewDescriptor.arrayLayerCount = 1;
            viewDescriptor.aspect = wgpu::TextureAspect::All;
            wgpu::TextureView targetView = texture.createView(viewDescriptor);
#ifndef WEBGPU_BACKEND_WGPU
            // We no longer need the texture, only its view
            // (NB: with wgpu-native, surface textures must not be manually released)
            wgpuTextureRelease(surfaceTexture.texture);
#endif // WEBGPU_BACKEND_WGPU
            view.ColorView = targetView;
            
            
            if (view.DepthView != nullptr) view.DepthView.release();
            view.DepthView = nullptr;
            if (target.DepthTexture)
            {
                wgpu::TextureViewDescriptor depthTextureViewDesc;
                depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
                depthTextureViewDesc.baseArrayLayer = 0;
                depthTextureViewDesc.arrayLayerCount = 1;
                depthTextureViewDesc.baseMipLevel = 0;
                depthTextureViewDesc.mipLevelCount = 1;
                depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
                depthTextureViewDesc.format = target.DepthFormat;
                WGPUTextureView depth_view = wgpuTextureCreateView(target.DepthTexture, &depthTextureViewDesc);
                view.DepthView = depth_view;
            }
        });

        w.system<GPU, RenderTargetViewComponent>().with<WindowComponent>().term_at(0).singleton().kind(flecs::PostUpdate).each([](GPU& gpu, RenderTargetViewComponent& rt)
        {
            wgpu::RenderPassColorAttachment colorAttachment;
            colorAttachment.view = rt.ColorView;
            colorAttachment.loadOp = wgpu::LoadOp::Clear;
            colorAttachment.storeOp = wgpu::StoreOp::Store;
            colorAttachment.clearValue = {0.5f, 0.5f, 0.5f, 1.0f};
            colorAttachment.resolveTarget = nullptr;
                
            wgpu::RenderPassDepthStencilAttachment depthAttachment;
            depthAttachment.view = rt.DepthView;
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
                
            wgpu::CommandEncoder encoder = gpu.Device.createCommandEncoder();
            wgpu::RenderPassEncoder pass = encoder.beginRenderPass(desc);
            pass.end();
            wgpu::CommandBuffer commands = encoder.finish();
            gpu.Device.getQueue().submit(1, &commands);

            pass.release();
            commands.release();
            encoder.release();
        });
        

        
        w.system<WindowComponent>("Window Poller")
            .kind(flecs::OnUpdate).each([](flecs::entity e, WindowComponent& window)
            {
                if (!window.WindowPtr) return;
                //glfwMakeContextCurrent(window.WindowPtr);
                //glfwPollEvents();
                
                if (glfwWindowShouldClose(window.WindowPtr))
                {
                    e.destruct();
                }
            });
        

        
        w.system<WindowRenderTargetComponent, RenderTargetViewComponent>("Window RT Presenter")
            .kind(flecs::PostUpdate).each([](WindowRenderTargetComponent& rt, RenderTargetViewComponent& v)
            {
                if (rt.SurfaceTexture.status != wgpu::SurfaceGetCurrentTextureStatus::Success) return;

                rt.Surface.present();
                v.ColorView.release();
                v.ColorView = nullptr;
                if (v.DepthView)
                {
                    v.DepthView.release();
                    v.DepthView = nullptr;
                }
            });
            
    }
};