//
// Created by Vinnie on 20/02/2025.
//

#pragma once

#include <webgpu/webgpu.h>
#include <webgpu/webgpu.hpp>

#include "GLFW/glfw3.h"
#include <glm/glm.hpp>
#include "Node.h"
#include "Mesh.h"

class Node;
using glm::mat4x4;
using glm::vec4;

// Avoid the "wgpu::" prefix in front of all WebGPU symbols



class Application {
public:
    Application(): device(nullptr), queue(nullptr), surface(nullptr), pipeline(nullptr) {
    } ;

    // Initialize everything and return true if it went all right
    bool Initialize();

    // Uninitialize everything that was initialized
    void Terminate();

    // Draw a frame and handle events
    void MainLoop();

    // Return true as long as the main loop should keep on running
    bool IsRunning();

    void OnResize();

private:
    wgpu::TextureView GetNextSurfaceTextureView();
    static wgpu::RequiredLimits GetRequiredLimits(wgpu::Adapter adapter);

private:
    bool InitializeWindowAndDevice();
    bool InitializeSurface();
    bool InitializeRenderPipeline();
    bool InitializeGeometry();
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
    void TerminateRenderPipeline();
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
        // We put here all the variables that are shared between init and main loop
    GLFWwindow *window;
    wgpu::Instance instance = nullptr;
    wgpu::Device device;
    wgpu::Queue queue;
    wgpu::Surface surface;
    wgpu::RenderPipeline pipeline = nullptr;
    wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Undefined;

    wgpu::Buffer uniformBuffer = nullptr;
    wgpu::Buffer perObjectUniformBuffer = nullptr;
    wgpu::PipelineLayout layout = nullptr;
    wgpu::BindGroupLayout bindGroupLayout = nullptr;
    wgpu::BindGroup bindGroup = nullptr;
    uint32_t uniformStride = 0;
    uint32_t perObjectUniformStride = 0;

    struct MyUniforms {
        mat4x4 projectionMatrix;
        mat4x4 viewMatrix;
        vec4 color;  // or float color[4]
        glm::vec3 cameraWorldPosition;
        float time;
        float _pad[3];
    };

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

    std::unique_ptr<Node> rootNode;

    void onMouseMove(double xpos, double ypos);
    void onMouseButton(int button, int action, [[]] int mods);
    void onScroll(double xoffset, double yoffset);
    void onKey(int key, int scancode, int action, int mods);

    void ImGuiDrawNode(Node* node);
};