#pragma once
#include <complex.h>
#include <iostream>
#include <glfw3webgpu/glfw3webgpu.h>

#include "Components.h"
#include "CrabTypes.h"
#include "flecs.h"
#include "Transform.h"
#include "Gfx/MeshVertex.h"
#include "GLFW/glfw3.h"
#include "webgpu/webgpu.hpp"

struct TransformModule
{
    TransformModule(flecs::world& world)
    {
        world.system<Transform, Transform>("Hierarchial Transforms")
            .cached().term_at(1).parent().cascade()
        .kind(flecs::OnUpdate).each([](Transform& t, Transform& parent)
        {
           t.ModelMatrix = parent.GetWorldModelMatrix() * t.GetLocalModelMatrix();
        });
    }
};

struct GPU
{
    wgpu::Instance Instance = nullptr;
    wgpu::Device Device = nullptr;
    std::unique_ptr<wgpu::ErrorCallback> ErrorCallbackhandle;
};

struct WGPUModule
{
    WGPUModule(flecs::world& w)
    {
        WGPUInstanceDescriptor instanceDesc{};
        auto Instance = wgpu::createInstance(instanceDesc);
        if (!Instance) {
            std::cerr << "Could not initialize WebGPU!" << std::endl;
        }

        std::cout << "Requesting adapter..." << std::endl;
        wgpu::RequestAdapterOptions adapterOpts = {};
        //surface = glfwGetWGPUSurface(wgpuInstance, window);
        //adapterOpts.compatibleSurface = surface;
        wgpu::Adapter adapter = Instance.requestAdapter(adapterOpts);
        std::cout << "Got adapter: " << adapter << std::endl;

        
        std::cout << "Requesting device..." << std::endl;
        wgpu::RequiredLimits requiredLimits = GetRequiredLimits(adapter);
        wgpu::DeviceDescriptor deviceDesc = {};
        deviceDesc.label = "My Device";
        deviceDesc.requiredFeatureCount = 0;
        deviceDesc.requiredLimits = &requiredLimits;
        deviceDesc.defaultQueue.nextInChain = nullptr;
        deviceDesc.defaultQueue.label = "The default queue";
        deviceDesc.deviceLostCallback = [](WGPUDeviceLostReason reason, char const* message, void*) {
            std::cout << "Device lost: reason " << reason;
            if (message) std::cout << " (" << message << ")";
            std::cout << std::endl;
        };
        auto Device = adapter.requestDevice(deviceDesc);
        std::cout << "Got device: " << Device << std::endl;

        auto ErrorCallbackhandle = Device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
            std::cout << "Uncaptured device error: type " << type;
            if (message) std::cout << " (" << message << ")";
            std::cout << std::endl;
        });

        w.set<GPU>({Instance, Device, std::move(ErrorCallbackhandle)});
    }

    
    wgpu::RequiredLimits GetRequiredLimits(wgpu::Adapter adapter)
    {
        wgpu::SupportedLimits supportedLimits;
        adapter.getLimits(&supportedLimits);

        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        int largestWidth = 0;
        int largestHeight = 0;
        for (int i = 0; i < monitorCount; i++) {
            int xpos;
            int ypos;
            int width;
            int height;
            glfwGetMonitorWorkarea(monitors[i], &xpos, &ypos, &width, &height);
            largestWidth += width;
            largestHeight += height;
        }

        wgpu::RequiredLimits requiredLimits = wgpu::Default;
        requiredLimits.limits.maxVertexAttributes = 6;
        requiredLimits.limits.maxVertexBuffers = 1;
        requiredLimits.limits.maxBufferSize = 10000000 * sizeof(MeshVertex);
        requiredLimits.limits.maxVertexBufferArrayStride = sizeof(MeshVertex);
        requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
        requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
        requiredLimits.limits.maxTextureDimension2D = supportedLimits.limits.maxTextureDimension2D;
        requiredLimits.limits.maxInterStageShaderComponents = 17;
        requiredLimits.limits.maxBindGroups = 4;
        requiredLimits.limits.maxUniformBuffersPerShaderStage = 3;
        requiredLimits.limits.maxUniformBufferBindingSize = 65536;
        requiredLimits.limits.maxDynamicUniformBuffersPerPipelineLayout = 1;
        requiredLimits.limits.maxTextureDimension1D = 4096;
        requiredLimits.limits.maxTextureDimension2D = 4096;
        requiredLimits.limits.maxTextureArrayLayers = 1;
        requiredLimits.limits.maxSampledTexturesPerShaderStage = 4;
        requiredLimits.limits.maxSamplersPerShaderStage = 4;
        requiredLimits.limits.maxBindingsPerBindGroup = 5;
        
        return requiredLimits;
    }
};




