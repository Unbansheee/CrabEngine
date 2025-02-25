//
// Created by Vinnie on 20/02/2025.
//

#pragma once

#include <array>
#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>
#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include "ApplicationLayer.h"
#include "imgui_multicontext_compositor.h"
#include "Node.h"

struct ImGuiContext;
using glm::mat4x4;
using glm::vec4;


// Avoid the "wgpu::" prefix in front of all WebGPU symbols
/*
class Application {
public:
    
    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();

    void OnResize();

public:
    wgpu::Instance GetInstance() const { return instance; }
    wgpu::Device GetDevice() const { return device; }
    wgpu::Queue GetQueue() const { return queue; }

    struct MyUniforms {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        vec4 color;  // or float color[4]
        glm::vec3 cameraWorldPosition;
        float time;
        float _pad[3];
    };
private:
    wgpu::TextureView GetNextSurfaceTextureView();
    static wgpu::RequiredLimits GetRequiredLimits(wgpu::Adapter adapter);

private:
    bool InitializeWindowAndDevice();
    bool InitializeSurface();
    //bool InitializeRenderPipeline();
    bool InitializeUniformBuffer();
    bool InitializeBindGroups();
    bool InitializeTextures();
    bool InitializeDepthBuffer();
    bool InitializeLightingUniforms();
    bool InitializeNodes();

    void TerminateTextures();
    void TerminateGeometry();
    void TerminateBindGroups();
    void TerminateUniformBuffer();
    //void TerminateRenderPipeline();
    void TerminateWindowAndDevice();
    void TerminateSurface();
    void TerminateDepthBuffer();
    void TerminateLightingUniforms();

    void UpdateProjectionMatrix();
    void UpdateViewMatrix();
    void UpdateDragInertia();
    void UpdateLightingUniforms();
    void UpdateNodes();

private:
    bool InitializeGUI();
    void UpdateGUI(wgpu::RenderPassEncoder renderPass);
    void TerminateGUI();

private:
    double previousFrameTime = 0.0;
    float deltaTime = 0.0f;

        // We put here all the variables that are shared between init and main loop
    GLFWwindow *window;
    wgpu::Instance instance = nullptr;
    wgpu::Device device = nullptr;
    wgpu::Queue queue = nullptr;
    wgpu::Surface surface = nullptr;
    //wgpu::RenderPipeline pipeline = nullptr;
    wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Undefined;

    wgpu::Buffer uniformBuffer = nullptr;
    wgpu::Buffer perObjectUniformBuffer = nullptr;
    wgpu::PipelineLayout layout = nullptr;
    wgpu::BindGroupLayout bindGroupLayout = nullptr;
    wgpu::BindGroup bindGroup = nullptr;
    uint32_t uniformStride = 0;
    uint32_t perObjectUniformStride = 0;

    std::unique_ptr<Node> rootNode;

    struct PerObjectUniforms {
        mat4x4 modelMatrix;
    };

    struct CameraState {
        // angles.x is the rotation of the camera around the global vertical axis, affected by mouse.x
        // angles.y is the rotation of the camera around its local horizontal axis, affected by mouse.y
        glm::vec2 angles = { 0.8f, 0.5f };
        // zoom is the position of the camera along its local forward axis, affected by the scroll wheel
        float zoom = -1.2f;
    };

    struct DragState {
        // Whether a drag action is ongoing (i.e., we are between mouse press and mouse release)
        bool active = false;
        // The position of the mouse at the beginning of the drag action
        glm::vec2 startMouse;
        // The camera state at the beginning of the drag action
        CameraState startCameraState;

        // Constant settings
        float sensitivity = 0.01f;
        float scrollSensitivity = 0.1f;

        // Inertia
        glm::vec2 velocity = {0.0, 0.0};
        glm::vec2 previousDelta;
        float inertia = 0.9f;
    };

    // Before Application's private attributes
    struct LightingUniforms {
        std::array<vec4, 2> directions;
        std::array<vec4, 2> colors;
        float hardness = 32.f;
        float kd = 1.0f;
        float ks = 0.5f;
        float normalStrength = 1.0f;
    };

    //static_assert(sizeof(MyUniforms) % 16 == 0);
    wgpu::Texture depthTexture = nullptr;
    wgpu::TextureView depthTextureView = nullptr;
    
    wgpu::Texture baseColorTexture = nullptr;
    wgpu::TextureView baseColorTextureView = nullptr;
    wgpu::Texture normalTexture = nullptr;
    wgpu::TextureView normalTextureView = nullptr;
    wgpu::Sampler sampler = nullptr;

    wgpu::Buffer lightingUniformBuffer = nullptr;
    LightingUniforms lightingUniforms{};
    bool lightingUniformsChanged = true;

    std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
    MyUniforms uniforms{};
    CameraState cameraState{};
    DragState dragState{};
    
    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, [[]] int mods);
    void onScroll(double xoffset, double yoffset);
    void onKey(int key, int scancode, int action, int mods);

    void ImGuiDrawNode(Node* node);
};
*/

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
    template<typename T>
    T* SetRootNode(std::unique_ptr<T> newRoot);
    Node* GetRootNode();
    void Update();
    bool ShouldClose() const;
    void Close();

    wgpu::Instance GetInstance() const { return wgpuInstance; }
    wgpu::Device GetDevice() const { return wgpuDevice; }
    wgpu::Queue GetQueue() const { return wgpuDeviceGetQueue(wgpuDevice); }

    static wgpu::RequiredLimits GetRequiredLimits(wgpu::Adapter adapter);
    
    struct MyUniforms {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        vec4 color;  // or float color[4]
        glm::vec3 cameraWorldPosition;
        float time;
        float _pad[3];
    };
    
protected:
    std::unique_ptr<Node> rootNode;
    DeltaTicker deltaTime;
    
    wgpu::Instance wgpuInstance = nullptr;
    wgpu::Device wgpuDevice = nullptr;

    std::unique_ptr<wgpu::ErrorCallback> errorCallbackHandle;

    Application(const Application &) = delete;
    Application & operator = (const Application &) = delete;
private:
    Application();
    ~Application();
};

template <typename T>
T* Application::SetRootNode(std::unique_ptr<T> newRoot)
{
    T* ptr = newRoot.get();
    rootNode = std::move(newRoot);
    return ptr;
}
