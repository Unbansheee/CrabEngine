//
// Created by Vinnie on 20/02/2025.
//

#pragma once

#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include "Nodes/Node.h"
#include "Core/SceneTree.h"

struct ImGuiContext;
using glm::mat4x4;
using glm::vec4;


struct DeltaTicker
{
public:
    float Tick(float newTime)
    {
        float delta = newTime - previousTime;
        previousTime = newTime;
        return delta;
    }

private:
    float previousTime = 0.0f;
};

class Application
{
public:
    static Application& Get()
    {
        static Application s;
        return s;
    }

    void Begin();
    void Update();
    bool ShouldClose() const;
    void Close();

    wgpu::Instance GetInstance() const { return wgpuInstance; }
    wgpu::Device GetDevice() const { return wgpuDevice; }
    wgpu::Queue GetQueue() const { return wgpuDeviceGetQueue(wgpuDevice); }

    static wgpu::RequiredLimits GetRequiredLimits(wgpu::Adapter adapter);

    SceneTree& GetSceneTree() {return sceneTree;}
    
    struct MyUniforms {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        vec4 color;  // or float color[4]
        glm::vec3 cameraWorldPosition;
        float time;
        float _pad[3];
    };
    
protected:
    SceneTree sceneTree;
    DeltaTicker deltaTime;
    bool bShouldClose = false;
    
    wgpu::Instance wgpuInstance = nullptr;
    wgpu::Device wgpuDevice = nullptr;

    std::unique_ptr<wgpu::ErrorCallback> errorCallbackHandle;

    Application(const Application &) = delete;
    Application & operator = (const Application &) = delete;
private:
    Application();
    ~Application();
};
