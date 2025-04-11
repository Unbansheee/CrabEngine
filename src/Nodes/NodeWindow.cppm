module;

#pragma once

//#include <webgpu/webgpu.hpp>
#include "ReflectionMacros.h"
#include "GLFW/glfw3.h"

export module Engine.Node.Window;
export import Engine.Node;
import Engine.GFX.Renderer;
import Engine.Object.Ref;
import Engine.WGPU;
import Engine.Node.Camera3D;
import Engine.Types;
import Engine.Resource.RuntimeTexture;

export class NodeWindow : public Node
{
    
    struct ResizeRequest
    {
        bool active = false;
        uint32_t width = 0;
        uint32_t height = 0;
    };

    
public:
    CRAB_CLASS(NodeWindow, Node)
    CLASS_FLAG(EditorVisible)

    
    void EnterTree() override;
    void Update(float dt) override;
    bool WantsToClose() const { return bCloseRequested; }
    void ExitTree() override;

    wgpu::TextureFormat GetSurfaceFormat() const { return surfaceFormat; }

    Renderer& GetRenderer() {return renderer;}
    wgpu::raii::Surface& GetSurface() {return surface;}
    wgpu::SurfaceTexture GetSurfaceTexture() const;

    void SetSurfaceDrawEnabled(bool enabled) {bShouldRenderNodesToSurface = enabled;}
    float GetAspectRatio() const;

    ObjectRef<NodeCamera3D> ActiveCamera;
    
protected:
    void InitializeRenderer();
    void TerminateSurface();
    void CreateIDPassTextures(uint32_t width, uint32_t height);
    
    virtual void RequestResize();
    virtual void OnKey(int key, int scancode, int action, int mods)
    {}
    virtual void OnScroll(double xoffset, double yoffset)
    {}
    virtual void OnMouseMove(double xpos, double ypos)
    {}
    virtual void OnMouseButton(int button, int action, int mods)
    {}

    wgpu::raii::TextureView GetCurrentTextureView() const;
    wgpu::raii::TextureView GetNextSurfaceTextureView() const;
    wgpu::raii::TextureView GetDepthTextureView() const;
    Vector2 GetWindowSize() const;

    void CreateSwapChain(uint32_t width, uint32_t height);
    void CreateDepthTexture(uint32_t width, uint32_t height);


protected:
    void ExecuteResize();

    bool bShouldRenderNodesToSurface = true;
    Renderer renderer;
    GLFWwindow* window = nullptr;
    wgpu::raii::Surface surface = nullptr;
    bool bCloseRequested = false;

    std::shared_ptr<RuntimeTextureResource> PickingPassTexture;
    
    wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat depthFormat = wgpu::TextureFormat::Depth24Plus;
    wgpu::raii::Texture m_depthTexture = nullptr;
    wgpu::raii::TextureView m_currentSurfaceView = nullptr;

    mutable wgpu::SurfaceTexture m_surfaceTexture;

    
    ResizeRequest resizeRequest;
};
