#pragma once
#include "Node.h"
#include "WeakRef.h"
#include "Renderer/Renderer.h"

class NodeWindow : public Node
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
    wgpu::Surface& GetSurface() {return surface;}
    wgpu::SurfaceTexture GetSurfaceTexture() const;

    void SetSurfaceDrawEnabled(bool enabled) {bShouldRenderNodesToSurface = enabled;}
    float GetAspectRatio() const;

    WeakRef<NodeCamera3D> ActiveCamera;
    
protected:
    void InitializeRenderer();
    void TerminateSurface();
    
    virtual void RequestResize();
    virtual void OnKey(int key, int scancode, int action, int mods)
    {}
    virtual void OnScroll(double xoffset, double yoffset)
    {}
    virtual void OnMouseMove(double xpos, double ypos)
    {}
    virtual void OnMouseButton(int button, int action, int mods)
    {}

    wgpu::TextureView GetCurrentTextureView();
    wgpu::TextureView GetNextSurfaceTextureView() const;
    wgpu::TextureView GetDepthTextureView() const;
    Vector2 GetWindowSize() const;

    void CreateSwapChain(uint32_t width, uint32_t height);
    void CreateDepthTexture(uint32_t width, uint32_t height);


protected:
    void ExecuteResize();

    bool bShouldRenderNodesToSurface = true;
    Renderer renderer;
    struct GLFWwindow* window = nullptr;
    wgpu::Surface surface = nullptr;
    bool bCloseRequested = false;
    
    wgpu::TextureFormat surfaceFormat = wgpu::TextureFormat::Undefined;
    wgpu::TextureFormat depthFormat = wgpu::TextureFormat::Depth24Plus;
    wgpu::Texture m_depthTexture = nullptr;
    wgpu::TextureView m_currentSurfaceView = nullptr;

    mutable wgpu::SurfaceTexture m_surfaceTexture;

    
    ResizeRequest resizeRequest;
};
